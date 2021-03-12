#include "pch.h"
#include "Steam.h"
#include "steam_hooks.h"
#include "constants.h"

#define HOOK(FUNC) installDetourHook(hooks, FUNC, #FUNC) 

void Steam::init()
{
	if(initialized || handle == NULL)
		return;

	logger->debug("Initializing Steam platform");

	HOOK(SteamInternal_FindOrCreateUserInterface);
	HOOK(SteamInternal_CreateInterface);
	HOOK(SteamApps);
	HOOK(SteamClient);

	logger->info("Steam platform was initialized");
	initialized = true;
}

void Steam::shutdown()
{
	if(!initialized)
		return;

	logger->debug("Shutting down Steam platform");


	for(auto& hook : hooks)
	{
		try
		{
			hook->unHook();
		} catch(std::exception& e)
		{
			logger->error("Failed to unhook: {}", e.what());
		}
	}
	hooks.clear();

	logger->debug("Steam platform was shut down");
}
