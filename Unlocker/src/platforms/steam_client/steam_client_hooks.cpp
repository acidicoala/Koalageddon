#include "pch.h"
#include "Logger.h"
#include "steam_client_hooks.h"

auto isBlacklistedInSteam(int dlcID)
{
	return vectorContains(config->platformRefs.Steam.blacklist, std::to_string(dlcID));
}

HOOK_SPEC(bool) IsAppDLCEnabled(PARAMS(int appID, int dlcID))
{
	auto enabled = !isBlacklistedInSteam(dlcID);
	logger->debug("IsAppDLCEnabled -> AppID: {}, DLC ID: {}. Enabled: {}", appID, dlcID, enabled);
	return enabled;
}

HOOK_SPEC(bool) IsSubscribedApp(PARAMS(int appID))
{
	auto subscribed = !isBlacklistedInSteam(appID);
	logger->debug("IsSubscribedApp -> AppID: {}. Subscribed: {}", appID, subscribed);
	return subscribed;
}

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

HOOK_SPEC(bool) GetDLCDataByIndex(PARAMS(int appID, int index, int* pDlcID , bool* pbAvailable, char* pchName, int bufferSize))
{
	auto result = PLH::FnCast(
		BasePlatform::trampolineMap[__func__],
		GetDLCDataByIndex
	)(ARGS(appID, index, pDlcID, pbAvailable, pchName, bufferSize));

	logger->info("GetDLCDataByIndex -> index: {}, DLC ID: {}, available: {}, name: '{}'", index, *pDlcID, *pbAvailable, pchName);

	// Steam magic happens here (3)
	*pbAvailable = !isBlacklistedInSteam(*pDlcID);

	return result;
}
