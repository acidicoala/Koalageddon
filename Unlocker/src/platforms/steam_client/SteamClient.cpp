#include "pch.h"
#include "SteamClient.h"
#include "steam_client_hooks.h"
#include "constants.h"
#include "PatternMatcher.h"

void SteamClient::fetchAndCachePatterns()
{
	logger->debug("Fetching SteamClient patterns");

	// Fetch offsets
	auto r = fetch(steamclient_patterns_url);

	if(r.status_code != 200)
	{
		logger->error("Failed to fetch SteamClient patterns: {} - {}", r.error.code, r.error.message);
		return;
	}

	// Cache offsets
	if(writeFileContents(PATTERNS_FILE_PATH, r.text))
		logger->info("SteamClient patterns were successfully fetched and cached");
	else
		logger->error("Failed to cache SteamClient patterns");
}

void SteamClient::readCachedPatterns()
{
	logger->debug("Reading SteamClient patterns from cache");

	auto text = readFileContents(PATTERNS_FILE_PATH);

	if(text.empty())
	{
		logger->error("No cached SteamClient patterns were found");
		return;
	}

	try
	{
		// Parse json into our vector
		json::parse(text, nullptr, true, true).get_to(patterns);
	} catch(json::exception& ex)
	{
		logger->error(L"Error parsing {}: {}", PATTERNS_FILE_PATH.wstring(), stow(ex.what()));
		return;
	}

	logger->info("SteamClient patterns were successfully read from file");
}

void SteamClient::installHook(void* hookedFunc, string funcName)
{
	static auto moduleInfo = getModuleInfo(handle);
	auto& [lpBaseOfDll, SizeOfImage, EntryPoint] = moduleInfo;

	auto& pattern = patterns[funcName];

	logger->debug("'{}' search pattern: '{}'", funcName, pattern);


	auto t1 = std::chrono::high_resolution_clock::now();
	auto origFuncAddress = PatternMatcher::scanInternal((PCSTR) lpBaseOfDll, SizeOfImage, pattern);
	auto t2 = std::chrono::high_resolution_clock::now();

	double elapsedTime = std::chrono::duration<double, std::milli>(t2 - t1).count();
	logger->debug("'{}' address: {}. Search time: {:.2f} ms", funcName, origFuncAddress, elapsedTime);

	if(origFuncAddress != nullptr)
		installDetourHook(hookedFunc, funcName.c_str(), origFuncAddress);
}

void SteamClient::installHooks()
{
	logger->info("steamclient.dll version: {}", getModuleVersion("steamclient.dll"));

#define HOOK(FUNC) installHook(FUNC, #FUNC)

#ifndef _WIN64 // Suppress the pointer size warnings on x64

	// We first try to hook Family Sharing functions,
	// since it is critical to hook them before they are called
	if(config->platformRefs.Steam.unlock_shared_library)
	{ 
		HOOK(SharedLibraryLockStatus);
		HOOK(SharedLibraryStopPlaying);
	}
	if(config->platformRefs.Steam.unlock_dlc && !config->platformRefs.Steam.replicate)
	{
		HOOK(IsAppDLCEnabled);
		HOOK(IsSubscribedApp);
		HOOK(GetDLCDataByIndex);
	}

#endif
}

void SteamClient::platformInit()
{
#ifndef _WIN64 // No point in x86-64 since Steam.exe is x86.
	logger->debug("Current process: {}, Steam process: {}", getCurrentProcessName(), config->platformRefs.Steam.process);
	if(!stringsAreEqual(getCurrentProcessName(), config->platformRefs.Steam.process, true))
	{
		logger->debug("Ignoring hooks since this is not a Steam process");
		return;
	}

	// Execute blocking operations in a new thread
	std::thread hooksThread([&]{
		std::thread fetchingThread([&]{ fetchAndCachePatterns(); });
		readCachedPatterns();

		if(patterns.size() == 0)
		{ // No cached patterns, hence we fetch them synchronously
			fetchingThread.join();
			readCachedPatterns();
		}
		else
		{ // Patterns were cached, hence we fetch them asynchronously
			fetchingThread.detach();
		}
		installHooks();
	});
	hooksThread.detach();

#endif
}

string SteamClient::getPlatformName()
{
	return STEAM_CLIENT_NAME;
}

LPCWSTR SteamClient::getModuleName()
{
	return STEAM_CLIENT;
}

Hooks& SteamClient::getPlatformHooks()
{
	return hooks;
}
