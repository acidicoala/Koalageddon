#include "pch.h"
#include "DLLMonitor.h"
#include "ntapi.h"
#include "constants.h"
#include "platforms/epic/Epic.h"
#include "platforms/steam/Steam.h"
#include <platforms/steam_client/SteamClient.h>
#include <platforms/ea/origin/Origin.h>
#include <platforms/ea/ea_desktop/EADesktop.h>
#include <platforms/uplay_r1/UplayR1.h>
#include <platforms/uplay_r2/UplayR2.h>


_LdrRegisterDllNotification LdrRegisterDllNotification = NULL;
_LdrUnregisterDllNotification LdrUnregisterDllNotification = NULL;

vector<unique_ptr<BasePlatform>> platforms;

bool getNtFunctions()
{
	HMODULE hNtDll = GetModuleHandle(L"ntdll.dll");
	if(hNtDll == NULL)
	{
		logger->error("Failed to get a handle for ntdll.dll module. Error code: {}", GetLastError());
		return false;
	}
	LdrRegisterDllNotification = (_LdrRegisterDllNotification) GetProcAddress(hNtDll, "LdrRegisterDllNotification");
	LdrUnregisterDllNotification = (_LdrUnregisterDllNotification) GetProcAddress(hNtDll, "LdrUnregisterDllNotification");

	if(!LdrRegisterDllNotification || !LdrUnregisterDllNotification)
	{
		logger->error("Some ntdll procedures were not found. Error code: {}", GetLastError());
		return false;
	}

	return true;
}

void CALLBACK dllCallback(ULONG NotificationReason, PLDR_DLL_NOTIFICATION_DATA NotificationData, PVOID Context)
{
	if(NotificationReason == LDR_DLL_NOTIFICATION_REASON_LOADED)
	{
		auto dllName = wstring(NotificationData->Loaded.BaseDllName->Buffer);

		// This log line sometimes crashes some games on Steam  >:(
		// logger->debug(L"DLL has been loaded: {}", dllName);

		if(dllName == EOSSDK)
		{
			logger->info(L"Epic Games DLL has been detected");
			platforms.push_back(make_unique<Epic>(NotificationData->Loaded.FullDllName->Buffer));
			platforms.back()->init();

		}
		else if(dllName == STEAMAPI)
		{
			logger->info(L"Steam DLL has been detected");
			platforms.push_back(make_unique<Steam>(NotificationData->Loaded.FullDllName->Buffer));
			platforms.back()->init();
		}
		else if(dllName == STEAM_CLIENT)
		{
			logger->info(L"SteamClient DLL has been detected");
			platforms.push_back(make_unique<SteamClient>(NotificationData->Loaded.FullDllName->Buffer));
			platforms.back()->init();
		}
		else if(dllName == ORIGINCLIENT)
		{
			logger->info(L"Origin DLL has been detected");
			platforms.push_back(make_unique<Origin>(NotificationData->Loaded.FullDllName->Buffer));
			platforms.back()->init();
		}
		else if(dllName == UPLAY_R1)
		{
			logger->info(L"UplayR1 DLL has been detected");
			platforms.push_back(make_unique<UplayR1>(NotificationData->Loaded.FullDllName->Buffer));
			platforms.back()->init();
		}
		/*else if(dllName == UPLAY_R2)
		{
			logger->info(L"UplayR2 DLL has been detected");
			platforms.push_back(make_unique<UplayR2>(NotificationData->Loaded.FullDllName->Buffer));
			platforms.back()->init();
		}*/
		else if(dllName == EA_DESKTOP)
		{
			logger->info(L"EA Desktop DLL has been detected");
			platforms.push_back(make_unique<EADesktop>(NotificationData->Loaded.FullDllName->Buffer));
			platforms.back()->init();
		}
	}
}


void checkLoadedDlls()
{
	logger->debug("Checking already loaded DLLs");

	HMODULE handle;

	if(handle = GetModuleHandle(EOSSDK))
	{
		logger->info("Epic DLL is already loaded");
		platforms.push_back(make_unique<Epic>(handle));
		platforms.back()->init();
	}
	else if(handle = GetModuleHandle(STEAMAPI))
	{
		logger->info("Steam DLL is already loaded");
		platforms.push_back(make_unique<Steam>(handle));
		platforms.back()->init();
	}
	else if(handle = GetModuleHandle(STEAM_CLIENT))
	{
		logger->info("SteamClient DLL is already loaded");
		platforms.push_back(make_unique<SteamClient>(handle));
		platforms.back()->init();
	}
	else if(handle = GetModuleHandle(ORIGINCLIENT))
	{
		logger->info(L"Origin DLL is already loaded");
		platforms.push_back(make_unique<Origin>(handle));
		platforms.back()->init();
	}
	else if(handle = GetModuleHandle(UPLAY_R1))
	{
		logger->info(L"UplayR1 DLL is already loaded");
		platforms.push_back(make_unique<UplayR1>(handle));
		platforms.back()->init();
	}
	/*else if(handle = GetModuleHandle(UPLAY_R2))
	{
		logger->info(L"UplayR2 DLL is already loaded");
		platforms.push_back(make_unique<UplayR2>(handle));
		platforms.back()->init();
	}*/
	else if(handle = GetModuleHandle(EA_DESKTOP))
	{
		logger->info(L"EA Desktop DLL is already loaded");
		platforms.push_back(make_unique<EADesktop>(handle));
		platforms.back()->init();
	}
	else
	{
		return;
	}

	// Don't need to dispose it, actually
	// CloseHandle(handle);
}

PVOID cookie = NULL;

void DLLMonitor::init()
{
	logger->debug("Initializing DLL monitor");

	if(!getNtFunctions())
	{
		return;
	}

	auto status = LdrRegisterDllNotification(0, &dllCallback, NULL, &cookie);
	if(status != STATUS_SUCCESS)
	{
		logger->error("Failed to register DLL notifications. Status code: {}", (unsigned long) status);
	}
	else
	{
		logger->debug("Registered DLL listener");
	}

	checkLoadedDlls();

	logger->debug("DLL monitor was successfully initialized");
}

void DLLMonitor::shutdown()
{
	logger->debug("Shutting down DLL monitor");


	for(auto& platform : platforms)
	{
		platform->shutdown();
	}

	// Deconstructor of each platform will do the unhooking
	platforms.clear();

	LdrUnregisterDllNotification(cookie);

	logger->debug("DLL monitor was successfully shut down");
}