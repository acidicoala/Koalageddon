#pragma once
#include "platforms/steam_client/SteamClient.h"

// DLC
HOOK_SPEC(bool) IsAppDLCEnabled(PARAMS(int appID, int dlcID));
HOOK_SPEC(bool) IsSubscribedApp(PARAMS(int appID));
HOOK_SPEC(bool) GetDLCDataByIndex(PARAMS(int appID, int index, int* pDlcID, bool* pbAvailable, char* pchName, int bufferSize));

// Family Sharing
HOOK_SPEC(bool) SharedLibraryLockStatus(PARAMS(void* mysteryInterface));
HOOK_SPEC(bool) SharedLibraryStopPlaying(PARAMS(void* mysteryInterface));
