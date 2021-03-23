#include "pch.h"
#include "steam_hooks.h"
#include "hook_util.h"
#include "steam_ordinals.h"
#include "platforms/steam/Steam.h"

auto& getSteamConfig()
{
	return config->platformRefs.Steam;
}

// forward declaration
void hookVirtualFunctions(void* interface, string version);

/////////////////////////
/// Virtual functions ///
/////////////////////////

bool __fastcall ISteamApps_BIsSubscribedApp(PARAMS(AppId_t appID))
{
	// Steam magic happens here
	auto subscribed = !vectorContains(getSteamConfig().blacklist, std::to_string(appID));
	logger->info("\tISteamApps_BIsSubscribedApp -> App ID: {}, Subscribed: {}", appID, subscribed);
	return subscribed;
}

bool __fastcall ISteamApps_BIsDlcInstalled(PARAMS(AppId_t appID))
{
	// Steam magic happens here (2)
	auto installed = !vectorContains(getSteamConfig().blacklist, std::to_string(appID));
	logger->info("\tISteamApps_BIsDlcInstalled -> App ID: {}, Installed: {}", appID, installed);
	return installed;
}

int __fastcall ISteamApps_GetDLCCount(PARAMS())
{
	logger->debug("ISteamApps_GetDLCCount");

	auto result = PLH::FnCast(
		BasePlatform::origVFuncMap[STEAM_APPS][ordinalMap[STEAM_APPS]["GetDLCCount"]],
		ISteamApps_GetDLCCount
	)(ARGS());

	logger->info("\tDLC count: {}", result);

	return result;
}

bool __fastcall ISteamApps_BGetDLCDataByIndex(PARAMS(int iDLC, AppId_t* pAppID, bool* pbAvailable, char* pchName, int cchNameBufferSize))
{
	logger->debug("\tISteamApps_BGetDLCDataByIndex -> index: {}", iDLC);

	auto result = PLH::FnCast(
		BasePlatform::origVFuncMap[STEAM_APPS][ordinalMap[STEAM_APPS]["BGetDLCDataByIndex"]],
		ISteamApps_BGetDLCDataByIndex
	)(ARGS(iDLC, pAppID, pbAvailable, pchName, cchNameBufferSize));

	logger->info("\tDLC Data -> index: {}, App ID: {}, available: {}, name: {}", iDLC, *pAppID, *pbAvailable, pchName);

	// Steam magic happens here (3)
	*pbAvailable = !vectorContains(getSteamConfig().blacklist, std::to_string(*pAppID));

	return result;
}

// I'm not sure if we need to hook this
EUserHasLicenseForAppResult __stdcall ISteamUser_UserHasLicenseForApp(PARAMS(CSteamID* steamID, AppId_t appID))
{
	logger->debug("UserHasLicenseForApp -> appID: {}", appID);

	return EUserHasLicenseForAppResult::k_EUserHasLicenseResultHasLicense;
}

ISteamApps* __fastcall ISteamClient_GetISteamApps(PARAMS(HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char* version))
{
	logger->debug("ISteamClient_GetISteamApps -> version: \"{}\"", version);

	auto interface = PLH::FnCast(
		BasePlatform::origVFuncMap[STEAM_CLIENT][ordinalMap[STEAM_CLIENT]["GetISteamApps"]],
		ISteamClient_GetISteamApps
	)(ARGS(hSteamUser, hSteamPipe, version));

	hookVirtualFunctions(interface, version);

	return interface;
}

///////////////////////
// Global functions ///
///////////////////////

void* SteamInternal_FindOrCreateUserInterface(HSteamUser hSteamUser, const char* version)
{
	logger->debug("SteamInternal_FindOrCreateUserInterface -> User: {}, Version: \"{}\"", hSteamUser, version);

	auto interface = PLH::FnCast(
		BasePlatform::trampolineMap[__func__],
		SteamInternal_FindOrCreateUserInterface
	)(hSteamUser, version);

	hookVirtualFunctions(interface, version);

	return interface;
}

void* SteamInternal_CreateInterface(const char* version)
{
	logger->debug("SteamInternal_CreateInterface -> Version: \"{}\"", version);

	auto interface = PLH::FnCast(
		BasePlatform::trampolineMap[__func__],
		SteamInternal_CreateInterface
	)(version);

	hookVirtualFunctions(interface, version);

	return interface;
}


// TODO: We don't know the interface version that games expects when it uses unversioned interface.
// For now let's assume earliest supported, but ideally we need to somehow determine exact version,
// since different SteamClient versions have different ordinals for interfaces.

void* SteamApps()
{
	logger->debug("SteamApps -> Version: {}", "UNVERSIONED");

	auto interface = PLH::FnCast(
		BasePlatform::trampolineMap["SteamApps"],
		SteamApps
	)();

	hookVirtualFunctions(interface, STEAM_APPS + "003");

	return interface;
}

void* SteamClient()
{
	logger->debug("SteamClient -> Version: {}", "UNVERSIONED");

	auto interface = PLH::FnCast(
		BasePlatform::trampolineMap[STEAM_CLIENT],
		SteamClient
	)();

	hookVirtualFunctions(interface, STEAM_CLIENT + "012");

	return interface;
}


void hookVirtualFunctions(void* interface, string version)
{
	logger->debug("hookVirtualFunctions -> interface: 0x{:X}, version: {}", (uint64_t) interface, version);

	if(interface == NULL) // Nothing to hook
		return; // This means that the game has tried to use an interface before initializing steam api

	if(startsWith(version, STEAM_CLIENT) && BasePlatform::origVFuncMap.count(STEAM_CLIENT) == 0)
	{
		logger->info("Hooking SteamClient interface: \"{}\"", version);

		const auto versionNumber = stoi(version.substr(STEAM_CLIENT.length()));
		if(versionNumber < 12)
		{
			logger->error("Not implemented version of SteamUser");
			return;
		}

		PLH::VFuncMap redirect = {
			{ ordinalMap[STEAM_CLIENT]["GetISteamApps"], (uint64_t) ISteamClient_GetISteamApps},
		};

		Steam::hooks.push_back(make_unique<PLH::VFuncSwapHook>((char*) interface, redirect, &BasePlatform::origVFuncMap[STEAM_CLIENT]));
		if(!Steam::hooks.back()->hook())
		{
			logger->error("Failed to hook");
			Steam::hooks.pop_back();
		}
	}


	if(startsWith(version, STEAM_APPS) && BasePlatform::origVFuncMap.count(STEAM_APPS) == 0)
	{
		logger->debug("Hooking SteamApps interface: \"{}\"", version);

		PLH::VFuncMap redirect = {
			{ordinalMap[STEAM_APPS]["BIsSubscribedApp"], (uint64_t) ISteamApps_BIsSubscribedApp},
			{ordinalMap[STEAM_APPS]["BIsDlcInstalled"], (uint64_t) ISteamApps_BIsDlcInstalled},
		};
		const auto versionNumber = stoi(version.substr(STEAM_APPS.length()));
		if(versionNumber >= 4)
		{
			redirect.emplace(ordinalMap[STEAM_APPS]["GetDLCCount"], (uint64_t) ISteamApps_GetDLCCount);
			redirect.emplace(ordinalMap[STEAM_APPS]["BGetDLCDataByIndex"], (uint64_t) ISteamApps_BGetDLCDataByIndex);
		}

		Steam::hooks.push_back(make_unique<PLH::VFuncSwapHook>((char*) interface, redirect, &BasePlatform::origVFuncMap[STEAM_APPS]));
		if(Steam::hooks.back()->hook())
		{
			logger->debug("Steam apps interface was successfully hooked");
		}
		else
		{
			logger->error("Failed to hook");
			Steam::hooks.pop_back();
		}
	}

	if(startsWith(version, STEAM_USER) && Steam::origVFuncMap.count(STEAM_USER) == 0)
	{
		logger->debug("Hooking SteamUser interface: \"{}\"", version);

		const auto versionNumber = stoi(version.substr(STEAM_USER.length()));
		if(versionNumber < 15)
		{
			logger->error("Not implemented version of SteamUser");
			return;
		}

		PLH::VFuncMap redirect = {
			{ordinalMap[STEAM_USER]["UserHasLicenseForApp"], (uint64_t) ISteamUser_UserHasLicenseForApp},
		};

		Steam::hooks.push_back(make_unique<PLH::VFuncSwapHook>((char*) interface, redirect, &Steam::origVFuncMap[STEAM_USER]));
		if(Steam::hooks.back()->hook())
		{
			logger->debug("Steam user interface was successfully hooked");
		}
		else
		{
			logger->error("Failed to hook");
			Steam::hooks.pop_back();
		}
	}
}
