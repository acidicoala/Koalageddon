#include "pch.h"
#include "BasePlatform.h"

BasePlatform::BasePlatform(const HMODULE handle)
{
	this->handle = handle;

	auto buffer = new WCHAR[MAX_PATH];
	GetModuleBaseName(GetCurrentProcess(), handle, buffer, MAX_PATH);
	this->moduleName = wtos(wstring(buffer));
	delete[] buffer;
}

BasePlatform::BasePlatform(const wstring& fullDllName)
{
	moduleName = wtos(fullDllName);

	auto handle = GetModuleHandle(fullDllName.c_str());
	if(handle == NULL)
	{
		logger->error(L"Failed to get a handle to the module: {}", fullDllName);
		return;
	}

	this->handle = handle;
}

void BasePlatform::installDetourHook(Hooks& hooks, void* hookedFunc, const char* funcName)
{
	static std::mutex mutex;
	const std::lock_guard lock(mutex);

	if(auto original_func_address = GetProcAddress(handle, funcName))
	{
		hooks.push_back(make_unique<Detour>
			((char*) original_func_address, (char*) hookedFunc, &trampolineMap[funcName], disassembler)
		);

		if(hooks.back()->hook())
		{
			logger->debug("Hooked \"{}\" via Detour.", funcName);
		}
		else
		{
			hooks.pop_back();
			logger->warn("Failed to hook \"{}\" via Detour. Trying with IAT.", funcName);
			installIatHook(hooks, hookedFunc, funcName);
		}
	}
}

void BasePlatform::installIatHook(Hooks& hooks, void* hookedFunc, const char* funcName)
{
	hooks.push_back(make_unique<PLH::IatHook>
		(moduleName, funcName, (char*) hookedFunc, &trampolineMap[funcName], L"")
	);

	if(hooks.back()->hook())
	{
		logger->debug("Hooked \"{}\" via IAT", funcName);
	}
	else
	{
		hooks.pop_back();
		logger->warn("Failed to hook \"{}\" via IAT.  Trying with EAT.", funcName);
		installEatHook(hooks, hookedFunc, funcName);
	}
}

void BasePlatform::installEatHook(Hooks& hooks, void* hookedFunc, const char* funcName)
{
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

