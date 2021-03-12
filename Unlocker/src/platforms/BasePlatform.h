#pragma once
#include "util.h"
#include "hook_util.h"

typedef vector<unique_ptr<PLH::IHook>> Hooks;

class BasePlatform
{
protected:
	string moduleName;
	HMODULE handle = NULL;
	bool initialized = false;

	// To learn the basics of each type of hook, you can consult this article by the 
	// author of the hooking library that is used by this projet: PolyHook 2.
	// The article was written for the v1 of the library, but the principles are the same in v2.
	// https://www.codeproject.com/articles/1100579/polyhook-the-cplusplus-x-x-hooking-library
	void installDetourHook(Hooks& hooks, void* hookedFunc, const char* funcName);
	void installIatHook(Hooks& hooks, void* hookedFunc, const char* funcName);
	void installEatHook(Hooks& hooks, void* hookedFunc, const char* funcName);

public:
	// we can safely store original functions from all platforms
	// in the corresponding single static container.
	inline static map<string, uint64_t> trampolineMap;
	inline static map<string, PLH::VFuncMap> origVFuncMap;

	BasePlatform() = delete;
	BasePlatform(const HMODULE handle);
	BasePlatform(const wstring& fullDllName);

	virtual void init() = 0;
	virtual void shutdown() = 0;
};
