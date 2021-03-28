#include "pch.h"
#include "UplayR1.h"
#include "uplay_r1_hooks.h"
#include "constants.h"

#define HOOK(FUNC) installDetourHook(FUNC, #FUNC);

void UplayR1::platformInit()
{
	HOOK(UPLAY_USER_IsOwned);
}

string UplayR1::getPlatformName()
{
	return UBISOFT_NAME;
}

LPCWSTR UplayR1::getModuleName()
{
	return UPLAY_R1;
}

Hooks& UplayR1::getPlatformHooks()
{
	return hooks;
}
