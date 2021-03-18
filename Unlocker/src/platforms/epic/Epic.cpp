#include "pch.h"
#include "Epic.h"
#include "util.h"
#include "constants.h"
#include "eos_hooks.h"

// Macro to avoid repetition
# define HOOK(FUNC) installDetourHook(FUNC, STR_##FUNC)

void Epic::platformInit()
{
	HOOK(EOS_Ecom_QueryOwnership);
	HOOK(EOS_Ecom_QueryEntitlements);
	HOOK(EOS_Ecom_GetEntitlementsCount);
	HOOK(EOS_Ecom_CopyEntitlementByIndex);
	HOOK(EOS_Ecom_Entitlement_Release);
}

string& Epic::getPlatformName()
{
	static string name = "Epic Games";
	return name;
}


Hooks& Epic::getPlatformHooks()
{
	return hooks;
}
