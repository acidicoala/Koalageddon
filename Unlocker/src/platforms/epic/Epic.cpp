#include "pch.h"
#include "Epic.h"
#include "constants.h"
#include "eos_hooks.h"

// Macro to avoid repetition
#define HOOK(FUNC) installDetourHook(FUNC, STR_##FUNC)

void Epic::platformInit()
{
	HOOK(EOS_Ecom_QueryOwnership);
	HOOK(EOS_Ecom_QueryEntitlements);
	HOOK(EOS_Ecom_GetEntitlementsCount);
	HOOK(EOS_Ecom_CopyEntitlementByIndex);
	HOOK(EOS_Ecom_Entitlement_Release);
}

string Epic::getPlatformName()
{
	return EPIC_GAMES_NAME;
}

LPCWSTR Epic::getModuleName()
{
	return EOSSDK;
}

Hooks& Epic::getPlatformHooks()
{
	return hooks;
}
