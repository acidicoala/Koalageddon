#include "pch.h"
#include "ea_desktop_hooks.h"
#include "EADesktop.h"
#include "constants.h"

#define GET_ORIGINAL_FUNC(FUNC) PLH::FnCast(BasePlatform::trampolineMap[mangled_##FUNC], FUNC);

/* An efficient algorithm to compare first N bytes of data with the needle string */
bool contains(const char* data, const std::string& needle, const int N = 8)
{
	const auto needleSize = needle.size();
	for(int i = 0; i < N; i++)
	{
		if(data[i] == '\0')
			return false; // Reached end of data

		size_t j;
		for(j = 0; j < needleSize; j++)
			if(data[i + j] != needle[j])
				break; // Mismatched needle char

		if(j == needleSize)
			return true; // Matched all needle chars
	}
	return false; // Reached N
}

template <typename T>
void scheduleForDeletion(T* data, int ms = 1000)
{
	std::thread([&](){
		Sleep(ms);
		delete data;
	}).detach();
}

HOOK_SPEC(const char*) QVector$data(PARAMS(void* mystery))
{
	static auto proxy = GET_ORIGINAL_FUNC(QVector$data);
	auto data = proxy(ARGS(mystery));

	static string langRequestId;
	static string needle("LSX");
	if(!contains(data, needle))
		return data; // Skip non LSX data

	string str(data);

	logger->debug("LSX intercepted:\n{}", str);

	if(saveLangRequestId(str, &langRequestId))
	{
		return data;
	}
	else
	{
		auto newStr = new string;
		if(modifyLangResponse(str, langRequestId, newStr) || modifyEntitlementReponse(str, newStr))
		{
			scheduleForDeletion(newStr); // Free the heap after 1s
			return newStr->c_str();
		}
		else
		{
			delete newStr;
			return data;
		}
	}
}
