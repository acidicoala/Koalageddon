#pragma once
#include "../BasePlatform.h"
#include "steamtypes.h"

class Steam : public BasePlatform
{
public:
	using BasePlatform::BasePlatform;

	inline static Hooks hooks;

	void init() override;
	void shutdown() override;
};
