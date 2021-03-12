#pragma once

#include "platforms/BasePlatform.h"
#include "util.h"

class Origin : public BasePlatform
{
public:
	using BasePlatform::BasePlatform;

	inline static Hooks hooks;

	void init() override;
	void shutdown() override;
};

