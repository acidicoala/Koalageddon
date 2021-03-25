#include "pch.h"
#include "Unlocker.h"
#include "Config.h"
#include "constants.h"
#include "DLLMonitor.h"
#include "ProcessHooker.h"
#include "UpdateChecker.h"

static bool initialized = false;

void Unlocker::init(HMODULE hModule)
{
	// Lock the thread to prevent init deadlock
	static std::mutex mutex;
	const std::lock_guard<std::mutex> lock(mutex);

	if(initialized)
		return;

	DisableThreadLibraryCalls(hModule);

	Config::init();

	Logger::init(UNLOCKER_NAME, true);

	logger->info("Unlocker v{}", VERSION);
	logger->info("Hooking into '{}'", getCurrentProcessName());

	UpdateChecker::checkForUpdates();

	DLLMonitor::init();
	ProcessHooker::init();

	//if(getCurrentProcessName() == Steam)

	initialized = true;
	logger->debug("Unlocker initialization complete");
}

void Unlocker::shutdown()
{
	static std::mutex mutex;
	const std::lock_guard<std::mutex> lock(mutex);

	if(!initialized)
		return;

	logger->debug("Unlocker shutting down");

	DLLMonitor::shutdown();
	ProcessHooker::shutdown();

	initialized = false;
	logger->debug("Unlocker was successfully shut down");
}
