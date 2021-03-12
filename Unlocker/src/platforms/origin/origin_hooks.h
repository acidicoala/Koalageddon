#pragma once
#include "util.h"
#include "hook_util.h"

constexpr auto mangled_encrypt = "?encrypt@SimpleEncryption@Crypto@Services@Origin@@QAE?AV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@ABV56@@Z";

using namespace tinyxml2;

extern XMLDocument entitlementsXML;

// The demangled signature is this:
// std::string __thiscall Origin::Services::Crypto::SimpleEncryption::encrypt(std::string const &)
// But it doesn't work. It needs strings to be pointers and some mystery argument. Why?
string* __fastcall encrypt(PARAMS(void* aes, string* message));
