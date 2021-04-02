#include "pch.h"
#include "Origin.h"
#include "origin_hooks.h"
#include "constants.h"

#define HOOK(FUNC) installDetourHook(FUNC, mangled_##FUNC)

void Origin::platformInit()
{
#ifndef _WIN64

	fetchEntitlementsAsync();

	if(stringsAreEqual(getCurrentProcessName(), config->platformRefs.Origin.process))
	{
		HOOK(encrypt);
	}

#endif
}

string Origin::getPlatformName()
{
	return ORIGIN_NAME;
}

LPCWSTR Origin::getModuleName()
{
	return ORIGINCLIENT;
}

Hooks& Origin::getPlatformHooks()
{
	return hooks;
}

