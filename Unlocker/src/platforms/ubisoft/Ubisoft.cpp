#include "pch.h"
#include "Ubisoft.h"
#include "ubisoft_hooks.h"

#define HOOK(FUNC) installDetourHook(hooks, FUNC, #FUNC);

void Ubisoft::init()
{
	if(initialized || handle == NULL)
		return;

	logger->debug("Initializing Ubisoft platform");

	HOOK(UPC_Init);
	HOOK(UPC_ProductListGet);
	HOOK(UPC_ProductListFree);

	logger->info("Ubisoft platform was initialized");
	initialized = true;
}

void Ubisoft::shutdown()
{
	if(!initialized)
		return;

	logger->debug("Shutting down Ubisoft platform");

	for(auto& hook : hooks)
	{
		hook->unHook();
	}
	hooks.clear();

	logger->debug("Ubisoft platform was shut down");
}
