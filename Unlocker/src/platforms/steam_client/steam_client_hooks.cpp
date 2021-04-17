#include "pch.h"
#include "Logger.h"
#include "steam_client_hooks.h"

#define GET_ORIGINAL_FUNC(NAME) PLH::FnCast(BasePlatform::trampolineMap[#NAME], NAME)

// Utility functions

auto isBlacklistedInSteam(int dlcID)
{
	return vectorContains(config->platformRefs.Steam.blacklist, std::to_string(dlcID));
}

auto isAppBlacklisted(int appID)
{
	return vectorContains(config->platformRefs.Steam.app_blacklist, std::to_string(appID));
}

/// DLC Unlocking hooks

HOOK_SPEC(bool) IsAppDLCEnabled(PARAMS(int appID, int dlcID))
{
	bool enabled;

	if(isAppBlacklisted(appID))
	{
		logger->debug("IsAppDLCEnabled -> Blacklisted AppID. Redirecting to original.");
		static auto original = GET_ORIGINAL_FUNC(IsAppDLCEnabled);
		enabled = original(ARGS(appID, dlcID));
	}
	else
	{
		enabled = !isBlacklistedInSteam(dlcID);
	}

	logger->debug("IsAppDLCEnabled -> AppID: {}, DLC ID: {}. Enabled: {}", appID, dlcID, enabled);
	return enabled;
}

HOOK_SPEC(bool) IsSubscribedApp(PARAMS(int appID))
{
	bool subscribed;
	if(isAppBlacklisted(appID))
	{
		logger->debug("GetDLCDataByIndex -> Blacklisted AppID. Redirecting to original.");
		static auto original = GET_ORIGINAL_FUNC(IsSubscribedApp);
		subscribed = original(ARGS(appID));
	}
	else
	{
		subscribed = !isBlacklistedInSteam(appID);
	}

	logger->debug("IsSubscribedApp -> AppID: {}. Subscribed: {}", appID, subscribed);
	return subscribed;
}

HOOK_SPEC(bool) GetDLCDataByIndex(PARAMS(int appID, int index, int* pDlcID, bool* pbAvailable, char* pchName, int bufferSize))
{
	static auto original = GET_ORIGINAL_FUNC(GetDLCDataByIndex);
	auto result = original(ARGS(appID, index, pDlcID, pbAvailable, pchName, bufferSize));

	if(isAppBlacklisted(appID))
	{
		logger->debug("GetDLCDataByIndex -> Blacklisted AppID. Skipping any modifications.");
	}
	else if(result) 
	{
		*pbAvailable = !isBlacklistedInSteam(*pDlcID);
	}

	logger->info("GetDLCDataByIndex -> index: {}, DLC ID: {}, available: {}, name: '{}'", index, *pDlcID, *pbAvailable, pchName);

	return result;
}

/// Family Sharing hooks

HOOK_SPEC(bool) SharedLibraryLockStatus(PARAMS(void* mysteryInterface))
{
	logger->debug("SharedLibraryLockStatus: {}", mysteryInterface);
	return true;
}

HOOK_SPEC(bool) SharedLibraryStopPlaying(PARAMS(void* mysteryInterface))
{
	logger->debug("SharedLibraryStopPlaying: {}", mysteryInterface);
	return true;
}