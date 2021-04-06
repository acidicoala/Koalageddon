#pragma once
#include "util.h"
#include "hook_util.h"
#include "platforms/ea/ea_util.h"

constexpr auto mangled_encrypt$SimpleEncryption = "?encrypt@SimpleEncryption@Crypto@Services@Origin@@QAE?AV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@ABV56@@Z";
constexpr auto mangled_decrypt$SimpleEncryption = "?decrypt@SimpleEncryption@Crypto@Services@Origin@@QAE?AV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@ABV56@@Z";


// The demangled signature is this:
// std::string __thiscall Origin::Services::Crypto::SimpleEncryption::encrypt(std::string const &)
// But it doesn't work. It needs a mystery argument. Why?
HOOK_SPEC(string*) encrypt$SimpleEncryption(PARAMS(void* mystery, string* message));
HOOK_SPEC(string*) decrypt$SimpleEncryption(PARAMS(void* mystery, string* message));

