#include "pch.h"
#include "UplayR1.h"

#include "uplay_r1_hooks.h"

#define HOOK(FUNC) installDetourHook(FUNC, #FUNC);

void UplayR1::platformInit()
{
	HOOK(UPLAY_USER_IsOwned);
}

string UplayR1::getPlatformName()
{
	return "Uplay R1";
}


Hooks& UplayR1::getPlatformHooks()
{
	return hooks;
}
