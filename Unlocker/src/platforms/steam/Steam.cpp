#include "pch.h"
#include "Steam.h"
#include "steam_hooks.h"
#include "constants.h"

#define HOOK(FUNC) installDetourHook(FUNC, #FUNC) 

void Steam::platformInit()
{
	if(config->platformRefs.Steam.unlock_dlc)
	{
		HOOK(SteamInternal_FindOrCreateUserInterface);
		HOOK(SteamInternal_CreateInterface);
		HOOK(SteamApps);
		HOOK(SteamClient);
	}
}

string Steam::getPlatformName()
{
	return STEAM_NAME;
}

LPCWSTR Steam::getModuleName()
{
	return STEAMAPI;
}

Hooks& Steam::getPlatformHooks()
{
	return hooks;
}
