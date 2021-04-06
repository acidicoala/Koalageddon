#include "pch.h"
#include "Origin.h"
#include "origin_hooks.h"
#include "constants.h"

#define HOOK(FUNC) installDetourHook(FUNC, mangled_##FUNC)

void Origin::platformInit()
{
	fetchEntitlementsAsync();

	if(stringsAreEqual(getCurrentProcessName(), config->platformRefs.Origin.process))
	{
		HOOK(encrypt$SimpleEncryption);
		HOOK(decrypt$SimpleEncryption);
	}
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

