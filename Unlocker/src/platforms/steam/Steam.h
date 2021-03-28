#pragma once
#include "../BasePlatform.h"
#include "steamtypes.h"

class Steam : public BasePlatform
{
protected:
	void platformInit() override;
	string getPlatformName() override;
	LPCWSTR getModuleName() override;
	Hooks& getPlatformHooks() override;
public:
	using BasePlatform::BasePlatform;

	inline static Hooks hooks;
};
