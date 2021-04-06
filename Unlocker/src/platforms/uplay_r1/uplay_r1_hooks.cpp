#include "pch.h"
#include "uplay_r1_hooks.h"
#include "platforms/uplay_r1/UplayR1.h"

#define GET_ORIGINAL_FUNC(FUNC) \
	static auto proxyFunc = PLH::FnCast(BasePlatform::trampolineMap[#FUNC], FUNC);

auto& getUplayR1Config()
{
	return config->platformRefs.UplayR1;
}

int UPLAY_USER_IsOwned(int aUplayId)
{
	GET_ORIGINAL_FUNC(UPLAY_USER_IsOwned);
	auto result = proxyFunc(aUplayId);

	auto isOwned = !vectorContains(getUplayR1Config().blacklist, std::to_string(aUplayId));

	logger->info(
		"UPLAY_USER_IsOwned -> aUplayId: {},\tisOwned: {}\t(legitimately owned: {})",
		aUplayId, isOwned, (bool) result
	);

	return isOwned;
}
