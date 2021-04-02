#include "pch.h"
#include "ProcessHooker.h"
#include "util.h"
#include "Logger.h"
#include "Config.h"
#include "constants.h"
#include "hook_util.h"

typedef LONG NTSTATUS;

// Undocumented functions

#pragma comment(lib,"ntdll.lib")
EXTERN_C NTSTATUS NTAPI NtSuspendProcess(HANDLE ProcessHandle);

#pragma comment(lib,"ntdll.lib")
EXTERN_C NTSTATUS NTAPI NtResumeProcess(HANDLE ProcessHandle);


uint64_t createProcTrampoline = NULL;

void inject(std::wstring wPID, uint64_t funcAddr)
{
	DWORD PID = std::stoi(wPID);

	// Determine the target executable's architecture to launch correponding injector
	bool is32 = is32bit(PID);

	auto installDir = getInstallDirPath();
	auto injectorPath = installDir / (is32 ? INJECTOR_32 : INJECTOR_64);
	auto unlockerPath = installDir / (is32 ? UNLOCKER_32 : UNLOCKER_64);

	// Validate paths
	if(!std::filesystem::exists(injectorPath))
	{
		logger->error(L"Injector exe was not found at: {}", injectorPath.c_str());
		return;
	}
	if(!std::filesystem::exists(unlockerPath))
	{
		logger->error(L"Unlocker dll was not found at: {}", unlockerPath.c_str());
		return;
	}

	auto args = fmt::format(L"\"{}\" {} \"{}\"", injectorPath.c_str(), wPID, unlockerPath.c_str());
	logger->debug(L"Starting Injector with cmdline: {}", args);

	// Need to make a non const copy since CreateProcessW might modify the argument
	auto size = args.size() + 1;
	auto cArgs = new WCHAR[size];
	wcscpy_s(cArgs, size, args.c_str());
	cArgs[size - 1] = '\0';

	// additional information
	STARTUPINFO si = {};
	PROCESS_INFORMATION pi = {};

	// We make sure to call the original function, to avoid recursively calling ourselves
	if(funcAddr == 0)
		funcAddr = (uint64_t) &CreateProcessW;

	auto Original_CreateProcessW = PLH::FnCast(funcAddr, &CreateProcessW);

	auto success = Original_CreateProcessW(
		injectorPath.c_str(),	// the path
		cArgs,					// Command line
		NULL,					// Process handle not inheritable
		NULL,					// Thread handle not inheritable
		FALSE,					// Set handle inheritance to FALSE
		0,						// No creation flags
		NULL,					// Use parent's environment block
		NULL,					// Use parent's starting directory 
		&si,					// Pointer to STARTUPINFO structure
		&pi						// Pointer to PROCESS_INFORMATION structure (removed extra parentheses)
	);

	if(!success)
	{
		logger->error("Failed to start Injector process. Error code: 0x{:X}", GetLastError());
		return;
	}

	delete[] cArgs;

	auto hProcess = pi.hProcess;
	if(hProcess == NULL)
	{
		logger->error("Process handle is NULL. Error code: 0x{:X}", GetLastError());
		return;
	}

	// Wait until injector process exits
	WaitForSingleObject(hProcess, INFINITE);

	DWORD exit_code;
	GetExitCodeProcess(hProcess, &exit_code);

	if(exit_code == 0)
	{
		logger->info(L"Successfully injected DLL into: {}", getProcessName(PID));
	}
	else
	{
		logger->error("Failed to inject DLL. Exit code: 0x{:X}", exit_code);
	}

	// Close process and thread handles
	CloseHandle(hProcess);

	if(pi.hThread != NULL)
		CloseHandle(pi.hThread);
}


void injectIfNecessary(wstring cmdLine, LPPROCESS_INFORMATION lpProcessInformation)
{
	std::wsmatch match;
	std::wregex pattern(LR"(\w+\.exe)");

	// Ignore system processes
	std::wregex windowsPattern(LR"(^"[A-Za-z]:\\[Ww][Ii][Nn][Dd][Oo][Ww][Ss]\\.*\.exe)");
	if(regex_search(cmdLine.cbegin(), cmdLine.cend(), match, windowsPattern))
	{
		logger->debug("Skipping injection into a Windows process");
		return;
	}

	if(!regex_search(cmdLine.cbegin(), cmdLine.cend(), match, pattern))
	{
		logger->debug("Failed to find exe name in the command line");
		return;
	}

	auto newProcName = wtos(match.str());

	// Iterate over terminate processes
	for(const auto& terminatedProcess : config->terminate)
	{
		if(terminatedProcess == newProcName)
		{
			// Kill the process if it is in the terminate list
			logger->warn("Terminating the process: {}", terminatedProcess);
			killProcess(lpProcessInformation->hProcess);
			return;
		}
	}

	// Iterate over ignored processes
	for(const auto& ignoredProcess : config->ignore)
	{
		if(ignoredProcess == newProcName)
		{
			// Don't inject the DLL, just let it run as usual
			logger->debug("Skipping injection for the globally ignored process: {}", ignoredProcess);
			return;
		}
	}

	// Iterate over platforms
	for(const auto& [key, platform] : config->platforms)
	{
		if(stringsAreEqual(getCurrentProcessName(), platform.process))
		{
			for(const auto& ignoredProcess : platform.ignore)
			{
				if(stringsAreEqual(newProcName, ignoredProcess))
				{
					// Do not inject since the process is ignored for this platforms
					logger->debug("Skipping injection since the new process is ignored for this platform");
					return;
				}
			}

			// Special Steam checks
			if(stringsAreEqual(config->platformRefs.Steam.process, platform.process))
			{
				// Steam->Uplay integration
				if(contains(wtos(cmdLine), "uplay_steam_mode"))
				{
					if(config->platformRefs.UplayR1.enabled && !config->platformRefs.UplayR1.replicate)
					{
						logger->debug("Skipping injection since Uplay replication is disabled");
						return;
					}
					else
					{
						break;
					}
				}

				if(!config->platformRefs.Steam.unlock_dlc)
				{
					logger->debug("Skipping injection since DLC unlocking in Steam is disabled");
					return;
				}
			}

			// This is a platform process
			if(!platform.replicate)
			{
				// Do not inject since platform is configured with disabled replication
				logger->debug("Skipping injection since {} replication is disabled", key);
				return;
			}
		}
	}

	// At this point we are sure that we want to inject the DLL

	inject(std::to_wstring(lpProcessInformation->dwProcessId), createProcTrampoline);
}

/**
 * We hook CreateProcessW to catch any new process that the current process creates,
 * so that we could inject unlocker there too. This is effectively a recursive
 * DLL injection which makes sure that we always reach the game executable,
 * even if it was started by a chain of launchers.
 */
BOOL WINAPI Hooked_CreateProcessW(
	LPCWSTR               lpApplicationName,
	LPWSTR                lpCommandLine,
	LPSECURITY_ATTRIBUTES lpProcessAttributes,
	LPSECURITY_ATTRIBUTES lpThreadAttributes,
	BOOL                  bInheritHandles,
	DWORD                 dwCreationFlags,
	LPVOID                lpEnvironment,
	LPCWSTR               lpCurrentDirectory,
	LPSTARTUPINFOW        lpStartupInfo,
	LPPROCESS_INFORMATION lpProcessInformation
)
{
	std::wstring appName(L"<unknown app name>");
	std::wstring cmdLine(L"<unknown command line>");

	if(lpApplicationName != NULL)
		appName = lpApplicationName;

	if(lpCommandLine != NULL)
		cmdLine = lpCommandLine;

	// Get the original function
	static auto Original_CreateProcessW = PLH::FnCast(createProcTrampoline, &CreateProcessW);

	// Call the original function
	auto result = Original_CreateProcessW(
		lpApplicationName,
		lpCommandLine,
		lpProcessAttributes,
		lpThreadAttributes,
		bInheritHandles,
		dwCreationFlags,
		lpEnvironment,
		lpCurrentDirectory,
		lpStartupInfo,
		lpProcessInformation
	);

	logger->debug(L"CreateProcessW -> PID: {}, command line: {}", lpProcessInformation->dwProcessId, cmdLine);

	if(result == FALSE)
		return result; // Some app made a messed up WINAPI call. Nothing to do for us here.

	NtSuspendProcess(lpProcessInformation->hProcess);

	injectIfNecessary(cmdLine, lpProcessInformation);

	NtResumeProcess(lpProcessInformation->hProcess);

	return result;
}
Detour detour((char*) &CreateProcessW, (char*) &Hooked_CreateProcessW, &createProcTrampoline, disassembler);

void ProcessHooker::init()
{
	logger->debug("Setting up process hooks");

	if(detour.hook())
	{
		logger->debug("Process hooks were successfully set up");
	}
	else
	{
		logger->error("Failed to hook CreateProcessW");
	}

}

void ProcessHooker::shutdown()
{
	logger->debug("Removing process hooks");

	detour.unHook();

	logger->debug("Process hooks were successfully removed");
}
