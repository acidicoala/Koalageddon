#include "pch.h"
#include "Injector.h"
#include "Logger.h"

// Source: https://github.com/saeedirha/DLL-Injector
int injectDLL(const int pid, path dllPath)
{
	auto wDllPath = dllPath.wstring();

	// 1. Get handle to the process
	auto processHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
	if(processHandle == NULL)
	{
		logger->error("Failed to open the target process. Error code: {}", GetLastError());
		return ERROR_PROCESS_OPEN;
	}

	// 2. Allocte memory in that process (tiny memory leak, no big deal)
	auto dllPathSize = 2 * (wDllPath.length() + 1);
	auto dllPathAddress = VirtualAllocEx(processHandle, NULL, dllPathSize, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	if(dllPathAddress == NULL)
	{
		logger->error("Failed to allocate memory in the target process. Error code: {}", GetLastError());
		CloseHandle(processHandle);
		return ERROR_TARGET_MALLOC;
	}

	// 3. Write DLL path into the newly allocated memory space
	auto writeSuccess = WriteProcessMemory(processHandle, dllPathAddress, wDllPath.c_str(), dllPathSize, NULL);
	if(!writeSuccess)
	{
		logger->error("Failed to write in memory of the target process. Error code: {}", GetLastError());
		CloseHandle(processHandle);
		return ERROR_TARGET_WRITE;
	}

	// 4. Create new thread in the target process
	auto threadHandle = CreateRemoteThread(processHandle, NULL, NULL, (LPTHREAD_START_ROUTINE) LoadLibraryW, dllPathAddress, NULL, NULL);
	if(threadHandle == NULL)
	{
		logger->error("Failed to create a remote thread in the target process. Error code: {}", GetLastError());
		CloseHandle(processHandle);
		return ERROR_REMOTE_THREAD;
	}

	CloseHandle(threadHandle);
	CloseHandle(processHandle);

	return OK;
}
