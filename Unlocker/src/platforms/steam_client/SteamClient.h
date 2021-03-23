#pragma once
#include "../BasePlatform.h"

class SteamClient : public BasePlatform
{
protected:
	const path OFFSETS_PATH = getCacheDirPath() / "steamclient-offsets.json";
	
	map<string, map<string, UINT32>> offsets;
	
	void fetchAndCacheOffsets();
	void readCachedOffsets();
	void installHooks();

	void platformInit() override;
	string getPlatformName() override;
	Hooks& getPlatformHooks() override;
public:
	using BasePlatform::BasePlatform;

	inline static Hooks hooks;
};
