#include "pch.h"
#include "EADesktop.h"
#include "ea_desktop_hooks.h"
#include "constants.h"

#define HOOK(FUNC) installDetourHook(FUNC, mangled_##FUNC);

void EADesktop::platformInit()
{
	fetchEntitlementsAsync();

	// Filter by process, because Qt5 is likely to be used by a process other than EA Desktop
	auto currentProcess = getCurrentProcessName();
	auto& eaDesktopProcess = config->platformRefs.EADesktop.process;
	logger->debug("Current process: {}, EA Desktop process: {}", currentProcess, eaDesktopProcess);
	if(stringsAreEqual(currentProcess, eaDesktopProcess))
	{
		HOOK(QVector$data);
	}
}

string EADesktop::getPlatformName()
{
	return EA_DESKTOP_NAME;
}

LPCWSTR EADesktop::getModuleName()
{
	return EA_DESKTOP;
}

Hooks& EADesktop::getPlatformHooks()
{
	return hooks;
}
