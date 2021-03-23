#include "pch.h"
#include "UplayR2.h"
#include "uplay_r2_hooks.h"

#define HOOK(FUNC) installDetourHook(FUNC, #FUNC);

void UplayR2::platformInit()
{
	HOOK(UPC_Init);
	HOOK(UPC_ProductListGet);
	HOOK(UPC_ProductListFree);
}

string UplayR2::getPlatformName()
{
	return "Uplay R2";
}


Hooks& UplayR2::getPlatformHooks()
{
	return hooks;
}
