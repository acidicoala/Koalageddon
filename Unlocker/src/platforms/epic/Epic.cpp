#include "pch.h"
#include "Epic.h"
#include "util.h"
#include "constants.h"
#include "eos_hooks.h"

// Macro to avoid repetition
# define HOOK(FUNC) installDetourHook(hooks, FUNC, STR_##FUNC)

void Epic::init()
{
	if(initialized || handle == NULL)
		return;

	logger->debug("Initializing Epic platform");

	HOOK(EOS_Ecom_QueryOwnership);
	HOOK(EOS_Ecom_QueryEntitlements);
	HOOK(EOS_Ecom_GetEntitlementsCount);
	HOOK(EOS_Ecom_CopyEntitlementByIndex);
	HOOK(EOS_Ecom_Entitlement_Release);

	logger->info("Epic platform was initialized");
	initialized = true;
}

void Epic::shutdown()
{
	if(!initialized)
		return;

	logger->debug("Shutting down Epic platform");

	for(auto& hook : hooks)
	{
		hook->unHook();
	}
	hooks.clear();

	logger->debug("Epic platform was shut down");
}
