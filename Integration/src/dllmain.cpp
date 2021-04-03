#include "pch.h"
#include "Logger.h"
#include "Config.h"
#include "constants.h"

using std::filesystem::path;

path thisDllPath;
HMODULE hUnlocker = NULL;
HMODULE hOriginal = NULL;

bool initialized = false;

void init(HMODULE hModule)
{
	static std::mutex m;
	const std::lock_guard<std::mutex> lock(m);
	if(initialized)
	{
		logger->warn("Already initialized");
	}
	Config::init();
	auto integrationDllStr = wtos(INTEGRATION);
	auto integrationStr = integrationDllStr.substr(0, integrationDllStr.size() - 4);
	Logger::init(integrationStr, true);
	logger->info("Integration v{}", VERSION);

	DisableThreadLibraryCalls(hModule);

	TCHAR buffer[MAX_PATH];
	auto result = GetModuleFileNameEx(GetCurrentProcess(), hModule, buffer, MAX_PATH);
	if(result == NULL)
	{
		logger->error("Failed to get path of current module. Error code: {}", GetLastError());
	}
	auto originalPath = absolute(buffer).parent_path() / "version_o.dll";
	hOriginal = LoadLibrary(originalPath.c_str());
	if(hOriginal == NULL)
	{
		logger->error(
			"Failed to load original library at: '{}'. Error code: {}",
			originalPath.string(), GetLastError()
		);
	}
	else
	{
		logger->info("Successfully loaded original DLL at: '{}'", originalPath.string());
	}

	auto currentProcess = getCurrentProcessName();
	logger->info("Current process: {}", currentProcess);

	for(const auto& [key, platform] : config->platforms)
	{
		if(stringsAreEqual(currentProcess, platform.process))
		{
			logger->info("Target platform detected: {}", platform.process);

			if(!platform.enabled)
			{
				logger->info("Skipping injection for {} platform since it is disabled", platform.process);
				return;
			}

			auto unlockerPath = getInstallDirPath() / UNLOCKER_DLL;
			logger->info("Unlocker path: '{}'", unlockerPath.string());

			hUnlocker = LoadLibrary(unlockerPath.wstring().c_str());
			if(hUnlocker == NULL)
			{
				logger->error("Failed to load the Unlocker. Error code: 0x{:X}", GetLastError());
			}
			else
			{
				logger->info("Successfully loaded the Unlocker");
			}
		}
	}
	logger->info("Initialization complete");
	initialized = true;
}

void shutdown()
{
	if(hUnlocker != NULL)
	{
		FreeLibrary(hUnlocker);
		hUnlocker = NULL;
		logger->info("Shutdown");
	}
	initialized = false;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{

	if(ul_reason_for_call == DLL_PROCESS_ATTACH)
		init(hModule);
	else if(ul_reason_for_call == DLL_PROCESS_DETACH)
		shutdown();

	return TRUE;
}

