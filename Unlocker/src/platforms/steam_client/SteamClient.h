#pragma once
#include "../BasePlatform.h"

class SteamClient : public BasePlatform
{
protected:
	const path PATTERNS_FILE_PATH = getCacheDirPath() / "steamclient-patterns.json";
	
	map<string, string> patterns;

	bool fetchAndCachePatterns() const;
	void readCachedPatterns();
	void installHook(void* hookedFunc, string funcName);
	void installHooks();

	void platformInit() override;
	string getPlatformName() override;
	LPCWSTR getModuleName() override;
	Hooks& getPlatformHooks() override;
public:
	using BasePlatform::BasePlatform;

	inline static Hooks hooks;
};
