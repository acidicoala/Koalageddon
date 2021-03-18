#include "pch.h"
#include "UplayR1.h"

#include "uplay_r1_hooks.h"

#define HOOK(FUNC) installDetourHook(FUNC, #FUNC);

void UplayR1::platformInit()
{
	HOOK(UPLAY_USER_IsOwned);
}

string& UplayR1::getPlatformName()
{
	static string name = "Uplay R1";
	return name;
}


Hooks& UplayR1::getPlatformHooks()
{
	return hooks;
}
