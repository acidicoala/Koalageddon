#pragma once
#include "util.h"

extern shared_ptr<spdlog::logger> logger;

namespace Logger
{

void init(string loggerName, bool truncate);

}
