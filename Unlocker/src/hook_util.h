#pragma once
#include "framework.h"

#ifdef _WIN64
typedef PLH::x64Detour Detour;
#else
typedef PLH::x86Detour Detour;
#endif

extern PLH::CapstoneDisassembler disassembler;

/**
 * By default, virtual functions are declared with __thiscall
 * convention, which is normal since they are class members.
 * But it presents an issue for us, since we cannot pass *this
 * pointer as a function argument. This is because *this
 * pointer is passed via register ECX in __thiscall
 * convention. Hence, to resolve this issue we declare our
 * hooked functions with __fastcall convention, to trick
 * the compiler into reading ECX & EDX registers as 1st
 * and 2nd function arguments respectively. Similarly, __fastcall
 * makes the compiler push the first argument into the ECX register,
 * which mimics the __thiscall calling convention. Register EDX
 * is not used anywhere in this case, but we still pass it along
 * to conform to the __fastcall convention. This all applies
 * to the x86 architecture.
 *
 * In x86-64 however, there is only one calling convention,
 * so __fastcall is simply ignored. However, RDX in this case
 * will store the 1st actual argument to the function, so we
 * have to omit it from the function signature.
 *
 * The macros below implement the above-mentioned considerations.
 */

#ifdef _WIN64
#define PARAMS(...) void* RCX, ##__VA_ARGS__
#define ARGS(...) RCX, ##__VA_ARGS__
#else
#define PARAMS(...) void* ECX, void* EDX, ##__VA_ARGS__
#define ARGS(...) ECX, EDX, ##__VA_ARGS__
#endif

#define HOOK_SPEC(TYPE) NOINLINE TYPE __fastcall
