#include "pch.h"
#include "Steam.h"
#include "steam_hooks.h"

#define HOOK(FUNC) installDetourHook(FUNC, #FUNC) 

void Steam::platformInit()
{
	HOOK(SteamInternal_FindOrCreateUserInterface);
	HOOK(SteamInternal_CreateInterface);
	HOOK(SteamApps);
	HOOK(SteamClient);
}

string Steam::getPlatformName()
{
	return "Steam";
}


Hooks& Steam::getPlatformHooks()
{
	return hooks;
}
