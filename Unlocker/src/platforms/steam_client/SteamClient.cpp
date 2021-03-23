#include "pch.h"
#include "SteamClient.h"
#include "steam_client_hooks.h"
#include "constants.h"

void SteamClient::fetchAndCacheOffsets()
{
	logger->debug("Fetching SteamClient offsets");

	// Fetch offsets
	cpr::Response r = cpr::Get(
		cpr::Url{ steamclient_offsets_url },
		cpr::Timeout{ 3 * 1000 } // 3s
	);

	if(r.status_code != 200)
	{
		logger->error("Failed to fetch SteamClient offsets: {} - {}", r.error.code, r.error.message);
		return;
	}

	// Cache offsets
	if(writeFileContents(OFFSETS_PATH, r.text))
		logger->info("SteamClient offsets were successfully fetched and cached");
	else
		logger->error("Failed to cached SteamClient offsets");
}

void SteamClient::readCachedOffsets()
{
	logger->debug("Reading SteamClient offsets from cache");

	auto text = readFileContents(OFFSETS_PATH.string());

	if(text.empty())
	{
		logger->error("No cached SteamClient offsets were found");
		return;
	}

	try
	{
		// Parse json into our vector
		json::parse(text, nullptr, true, true).get_to(offsets);
	} catch(json::exception& ex)
	{
		logger->error("Error parsing SteamClient json: {}", ex.what());
		return;
	}

	logger->info("SteamClient offsets were successfully read from file");
}

void SteamClient::installHooks()
{
	map<string, UINT32> latestOffsets;
	auto dllVersion = getModuleVersion("steamclient.dll");

	logger->info("steamclient.dll version: {}", dllVersion);

	try
	{
		latestOffsets = offsets.at(dllVersion);
	} catch(std::out_of_range&)
	{
		// Steamclient has been updated, but offsets have not been updated yet
		// Try again next day ¯\_(ツ)_/¯
		logger->error("Unsupported Steamclient version. Wait for acidicoala to add support for it.");
		return;
	}
#define HOOK(FUNC) installDetourHook(FUNC, #FUNC, (void*)(latestOffsets[#FUNC] + (UINT32) handle))

#ifndef _WIN64 // Suppress the pointer size warnings on x64
	if(!config->platformRefs.Steam.replicate)
	{
		HOOK(IsAppDLCEnabled);
		HOOK(IsSubscribedApp);
		HOOK(GetDLCDataByIndex);
	}

	if(config->platformRefs.Steam.unlock_shared_library)
	{
		HOOK(SharedLibraryLockStatus);
		HOOK(SharedLibraryStopPlaying);
	}
#endif

}

void SteamClient::platformInit()
{
#ifndef _WIN64 // No point in x86-64 since Steam.exe is x86.

	if(stringsAreEqual(getCurrentProcessName(), config->platformRefs.Steam.process, true))
	{
		logger->debug("Ignoring hooks since this is not a Steam process");
		return;
	}

	// Execute blocking operations in a new thread
	std::thread fetchingThread([&]{
		logger->debug("Steamclient hooks: thread started");

		/*
		Here we read cached offsets twice since the https request might take too long,
		and it will be too late to patch steam. Hence we also read them before fetching
		since in most cases, valid offsets will be cached.
		*/

		readCachedOffsets(); // If there are cached offsets
		fetchAndCacheOffsets();
		readCachedOffsets(); // If there are no cached offsets
		installHooks();

		logger->debug("Steamclient hooks: thread finished");
	});
	fetchingThread.detach();

#endif
}

string SteamClient::getPlatformName()
{
	return "SteamClient";
}

Hooks& SteamClient::getPlatformHooks()
{
	return hooks;
}
