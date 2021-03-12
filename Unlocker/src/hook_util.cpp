#include "pch.h"
#include "hook_util.h"

#ifdef _WIN64
PLH::CapstoneDisassembler disassembler(PLH::Mode::x64);
#else
PLH::CapstoneDisassembler disassembler(PLH::Mode::x86);
#endif
