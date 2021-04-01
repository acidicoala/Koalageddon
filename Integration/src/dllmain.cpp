#include "pch.h"
#include "Logger.h"
#include "Config.h"
#include "constants.h"

using std::filesystem::path;

path thisDllPath;
HMODULE hUnlocker = NULL;

void init(HMODULE hModule)
{
	Logger::init("Integration", true);
	logger->info("Integration v{}", VERSION);

	DisableThreadLibraryCalls(hModule);

	auto currentProcess = getCurrentProcessName();
	logger->debug("Current process: {}", currentProcess);

	for(const auto& [key, platform] : config->platforms)
	{
		if(stringsAreEqual(currentProcess, platform.process, true))
		{
			logger->info("Target platform detected: {}", platform.process);

			if(!platform.enabled)
			{
				logger->info("Skipping injection for {} platform since it is disabled", platform.process);
				return;
			}

			auto unlockerPath = getInstallDirPath() / UNLOCKER_DLL;
			logger->debug("Unlocker path: '{}'", unlockerPath.string());
			
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
}

void shutdown()
{
	if(hUnlocker != NULL)
		FreeLibrary(hUnlocker);

}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{

	if(ul_reason_for_call == DLL_PROCESS_ATTACH)
		init(hModule);
	else if(ul_reason_for_call == DLL_PROCESS_DETACH)
		shutdown();

	return TRUE;
}

