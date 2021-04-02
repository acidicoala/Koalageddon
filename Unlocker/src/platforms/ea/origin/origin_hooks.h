#pragma once
#include "util.h"
#include "hook_util.h"
#include "platforms/ea/ea_util.h"

constexpr auto mangled_encrypt = "?encrypt@SimpleEncryption@Crypto@Services@Origin@@QAE?AV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@ABV56@@Z";

#ifndef _WIN64

// The demangled signature is this:
// std::string __thiscall Origin::Services::Crypto::SimpleEncryption::encrypt(std::string const &)
// But it doesn't work. It needs a mystery argument. Why?
string* __fastcall encrypt(PARAMS(void* mystery, string* message));

#endif