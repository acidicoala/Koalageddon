#pragma once
#include <codeanalysis\warnings.h>

#include <Logger.h>
#include <Config.h>

// Disable 3rd party library warnings
#pragma warning(push)
#pragma warning(disable: ALL_CODE_ANALYSIS_WARNINGS)

#include <polyhook2/Detour/x64Detour.hpp>
#include <polyhook2/Detour/x86Detour.hpp>
#include "polyhook2/PE/EatHook.hpp"
#include "polyhook2/PE/IatHook.hpp"
#include <polyhook2/CapstoneDisassembler.hpp>
#include <polyhook2/Virtuals/VFuncSwapHook.hpp>
#include <polyhook2/Virtuals/VTableSwapHook.hpp>

#pragma warning(pop)
