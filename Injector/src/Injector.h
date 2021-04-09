#pragma once
#include "framework.h"
#include "util.h"

enum ExitCode
{
	OK,
	ERROR_INVALID_ARGUMENTS,
	ERROR_PROCESS_OPEN,
	ERROR_TARGET_MALLOC,
	ERROR_TARGET_WRITE,
	ERROR_KERNEL_HANDLE,
	ERROR_PROC_ADDRESS,
	ERROR_REMOTE_THREAD
};

int injectDLL(const int pid, path DLL_Path);
