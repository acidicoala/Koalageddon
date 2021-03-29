#include "pch.h"
#include "BasePlatform.h"

void BasePlatform::init()
{
	if(initialized || handle == NULL)
		return;

	logger->debug("Initializing {} platform", getPlatformName());

	platformInit();

	logger->info("{} platform was initialized", getPlatformName());
	initialized = true;
}

BasePlatform::BasePlatform(const HMODULE handle)
{
	this->handle = handle;

	auto buffer = new WCHAR[MAX_PATH];
	GetModuleBaseName(GetCurrentProcess(), handle, buffer, MAX_PATH);
	this->moduleName = wtos(buffer);
	delete[] buffer;
}

BasePlatform::BasePlatform(const wstring& fullDllName)
{
	this->moduleName = wtos(fullDllName);

	auto handle = GetModuleHandle(fullDllName.c_str());
	if(handle == NULL)
	{
		logger->error(L"Failed to get a handle to the module: {}", fullDllName);
		return;
	}

	this->handle = handle;
}

// Destructor
void BasePlatform::shutdown()
{
	if(!initialized)
		return;

	logger->debug("Shutting down {} platform", getPlatformName());

	auto& hooks = getPlatformHooks();

	if(GetModuleHandle(getModuleName()))
	{ // Unhook only if the module is still loaded
		for(auto& hook : hooks)
		{ // Deletes the underlying object (calls destructor, which calls unHook())
			hook.reset();
		}
	}
	else
	{
		logger->debug(L"Skipping unhooking because the {} is unloaded", getModuleName());
		for(auto& hook : hooks)
		{ // Does not delete the object, hence unHook() is not called
			hook.release();
		}
	}
	hooks.clear();

	logger->debug("{} platform was shut down", getPlatformName());
}

void BasePlatform::installDetourHook(void* hookedFunc, const char* funcName, void* funcAddress)
{
	logger->debug("Hooking {} at {}", funcName, funcAddress);

	auto& hooks = getPlatformHooks();

	hooks.push_back(make_unique<Detour>
		((char*) funcAddress, (char*) hookedFunc, &trampolineMap[funcName], disassembler)
	);

	if(hooks.back()->hook())
	{
		logger->info("Hooked '{}' via Detour.", funcName);
	}
	else
	{
		hooks.pop_back();
		logger->error("Failed to hook '{}' via Detour.", funcName);
	}
}

void BasePlatform::installDetourHook(void* hookedFunc, const char* funcName)
{
	auto& hooks = getPlatformHooks();

	if(auto original_func_address = GetProcAddress(handle, funcName))
	{
		hooks.push_back(make_unique<Detour>
			((char*) original_func_address, (char*) hookedFunc, &trampolineMap[funcName], disassembler)
		);

		if(hooks.back()->hook())
		{
			logger->info("Hooked '{}' via Detour.", funcName);
		}
		else
		{
			hooks.pop_back();
			logger->warn("Failed to hook '{}' via Detour. Trying with IAT.", funcName);
			installIatHook(hookedFunc, funcName);
		}
	}
	else
	{
		logger->error("Failed to find address of '{}' procedure", funcName);
	}
}

void BasePlatform::installIatHook(void* hookedFunc, const char* funcName)
{
	auto& hooks = getPlatformHooks();

	hooks.push_back(make_unique<PLH::IatHook>
		(moduleName, funcName, (char*) hookedFunc, &trampolineMap[funcName], L"")
	);

	if(hooks.back()->hook())
	{
		logger->debug("info '{}' via IAT", funcName);
	}
	else
	{
		hooks.pop_back();
		logger->warn("Failed to hook \"{}\" via IAT.  Trying with EAT.", funcName);
		installEatHook(hookedFunc, funcName);
	}
}

void BasePlatform::installEatHook(void* hookedFunc, const char* funcName)
{
	auto& hooks = getPlatformHooks();

	hooks.push_back(make_unique<PLH::EatHook>
		(funcName, stow(moduleName), (char*) hookedFunc, &trampolineMap[funcName])
	);

	if(hooks.back()->hook())
	{
		logger->debug("Hooked \"{}\" via EAT", funcName);
	}
	else
	{
		hooks.pop_back();
		logger->error("Failed to hook \"{}\" via EAT.", funcName);
	}
}

