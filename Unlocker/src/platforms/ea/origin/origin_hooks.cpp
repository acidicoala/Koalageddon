#include "pch.h"
#include "origin_hooks.h"
#include "Origin.h"

#define GET_ORIGINAL_FUNC(FUNC) PLH::FnCast(BasePlatform::trampolineMap[mangled_##FUNC], FUNC);

string langRequestId;

HOOK_SPEC(string*) encrypt$SimpleEncryption(PARAMS(void* mystery, string* message))
{
	false
		|| modifyLangResponse(*message, langRequestId, message)
		|| modifyEntitlementReponse(*message, message);

	static auto original = GET_ORIGINAL_FUNC(encrypt$SimpleEncryption);
	return original(ARGS(mystery, message));
}

HOOK_SPEC(string*) decrypt$SimpleEncryption(PARAMS(void* mystery, string* message))
{
	static auto original = GET_ORIGINAL_FUNC(decrypt$SimpleEncryption);
	auto decrypted = original(ARGS(mystery, message));

	saveLangRequestId(*decrypted, &langRequestId);

	return decrypted;
}

