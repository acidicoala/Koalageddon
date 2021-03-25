#pragma once
#include "framework.h"

namespace PatternMatcher
{

NOINLINE PVOID scanInternal(PCSTR pMemory, size_t length, string pattern);

}