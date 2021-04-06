#pragma once
#include "util.h"
#include "hook_util.h"
#include "platforms/ea/ea_util.h"

constexpr auto mangled_QVector$data = "?data@?$QVector@VQXmlStreamAttribute@@@@QEBAPEBVQXmlStreamAttribute@@XZ";

HOOK_SPEC(const char*) QVector$data(PARAMS(void* mystery));
