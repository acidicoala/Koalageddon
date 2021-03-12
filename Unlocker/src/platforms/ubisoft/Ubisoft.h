#pragma once
#include "platforms/BasePlatform.h"
class Ubisoft : public BasePlatform
{
public:
	using BasePlatform::BasePlatform;

	inline static Hooks hooks;

	void init() override;
	void shutdown() override;
};

