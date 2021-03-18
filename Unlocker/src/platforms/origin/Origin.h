#pragma once

#include "platforms/BasePlatform.h"
#include "util.h"

class Origin : public BasePlatform
{
protected:
	void platformInit() override;
	string& getPlatformName() override;
	Hooks& getPlatformHooks() override;
public:
	using BasePlatform::BasePlatform;

	inline static Hooks hooks;
};

