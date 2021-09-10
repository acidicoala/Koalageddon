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

_LdrRegisterDllNotification LdrRegisterDllNotification = nullptr;
_LdrUnregisterDllNotification LdrUnregisterDllNotification = nullptr;

auto& platforms = *new vector<unique_ptr<BasePlatform>>();

bool getNtFunctions() {
	const HMODULE hNtDll = GetModuleHandle(L"ntdll.dll");
	if (hNtDll == nullptr) {
		logger->error("Failed to get a handle for ntdll.dll module. Error code: {}", GetLastError());
		return false;
	}
	LdrRegisterDllNotification = reinterpret_cast<_LdrRegisterDllNotification>(GetProcAddress(hNtDll, "LdrRegisterDllNotification"));
	LdrUnregisterDllNotification = reinterpret_cast<_LdrUnregisterDllNotification>(GetProcAddress(hNtDll, "LdrUnregisterDllNotification"));

	if (!LdrRegisterDllNotification || !LdrUnregisterDllNotification) {
		logger->error("Some ntdll procedures were not found. Error code: {}", GetLastError());
		return false;
	}

	return true;
}

void CALLBACK dllCallback(ULONG NotificationReason, PLDR_DLL_NOTIFICATION_DATA NotificationData, PVOID Context) {
	if (NotificationReason == LDR_DLL_NOTIFICATION_REASON_LOADED) {
		auto dllName = wstring(NotificationData->Loaded.BaseDllName->Buffer);

		// This log line sometimes crashes some games on Steam  >:(
		// logger->debug(L"DLL has been loaded: {}", dllName);

		if (dllName == EOSSDK) {
			logger->info(L"Epic Games DLL has been detected");
			platforms.push_back(make_unique<Epic>(NotificationData->Loaded.FullDllName->Buffer));
			platforms.back()->init();

		} else if (dllName == STEAMAPI) {
			logger->info(L"Steam DLL has been detected");
			platforms.push_back(make_unique<Steam>(NotificationData->Loaded.FullDllName->Buffer));
			platforms.back()->init();
		} else if (dllName == STEAM_CLIENT) {
			logger->info(L"SteamClient DLL has been detected");
			platforms.push_back(make_unique<SteamClient>(NotificationData->Loaded.FullDllName->Buffer));
			platforms.back()->init();
		} else if (dllName == ORIGINCLIENT) {
			logger->info(L"Origin DLL has been detected");
			platforms.push_back(make_unique<Origin>(NotificationData->Loaded.FullDllName->Buffer));
			platforms.back()->init();
		} else if (dllName == UPLAY_R1) {
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
		else if (dllName == EA_DESKTOP) {
			// Additional filter since same dll is loaded by origin too
			if (stringsAreEqual(getCurrentProcessName(), config->platformRefs.EADesktop.process)) {
				logger->info(L"EA Desktop DLL has been detected");
				platforms.push_back(make_unique<EADesktop>(NotificationData->Loaded.FullDllName->Buffer));
				platforms.back()->init();
			}
		}
	}
}


void checkLoadedDlls() {
	logger->debug("Checking already loaded DLLs");

	HMODULE handle;

	if ((handle = GetModuleHandle(EOSSDK))) {
		logger->info("Epic DLL is already loaded");
		platforms.push_back(make_unique<Epic>(handle));
		platforms.back()->init();
	} else if ((handle = GetModuleHandle(STEAMAPI))) {
		logger->info("Steam DLL is already loaded");
		platforms.push_back(make_unique<Steam>(handle));
		platforms.back()->init();
	} else if ((handle = GetModuleHandle(STEAM_CLIENT))) {
		logger->info("SteamClient DLL is already loaded");
		platforms.push_back(make_unique<SteamClient>(handle));
		platforms.back()->init();
	} else if ((handle = GetModuleHandle(ORIGINCLIENT))) {
		logger->info(L"Origin DLL is already loaded");
		platforms.push_back(make_unique<Origin>(handle));
		platforms.back()->init();
	} else if ((handle = GetModuleHandle(UPLAY_R1))) {
		logger->info(L"UplayR1 DLL is already loaded");
		platforms.push_back(make_unique<UplayR1>(handle));
		platforms.back()->init();
	} else if ((handle = GetModuleHandle(EA_DESKTOP))) {
		if (stringsAreEqual(getCurrentProcessName(), config->platformRefs.EADesktop.process)) {
			logger->info(L"EA Desktop DLL is already loaded");
			platforms.push_back(make_unique<EADesktop>(handle));
			platforms.back()->init();
		}
	} /*else if(handle = GetModuleHandle(UPLAY_R2)) {
		logger->info(L"UplayR2 DLL is already loaded");
		platforms.push_back(make_unique<UplayR2>(handle));
		platforms.back()->init();
	}*/

}

PVOID cookie = nullptr;

void DLLMonitor::init() {
	logger->debug("Initializing DLL monitor");

	if (!getNtFunctions()) {
		return;
	}

	const auto status = LdrRegisterDllNotification(0, &dllCallback, nullptr, &cookie);
	if (status != STATUS_SUCCESS) {
		logger->error(
			"Failed to register DLL notifications. Status code: {}",
			static_cast<unsigned long>(status)
		);
	} else {
		logger->debug("Registered DLL listener");
	}

	checkLoadedDlls();

	logger->debug("DLL monitor was successfully initialized");
}

void DLLMonitor::shutdown() {
	logger->debug("Shutting down DLL monitor");


	for (const auto& platform : platforms) {
		platform->shutdown();
	}

	// Destructor of each platform will do the unhooking
	platforms.clear();

	LdrUnregisterDllNotification(cookie);

	logger->debug("DLL monitor was successfully shut down");
}
