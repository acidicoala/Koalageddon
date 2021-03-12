#include "steamtypes.h"

// TODO: This only works if SteamAPI dll is loaded only once.
// Multiple loaded SteamAPI dlls will result in undefined behaviour.
// One solution is to store all global objects into a map,
// with key being the handle to the dll.


// Safe interfaces
void* S_CALLTYPE SteamInternal_FindOrCreateUserInterface(HSteamUser hSteamUser, const char* pszVersion);
void* S_CALLTYPE SteamInternal_CreateInterface(const char* pszVersion);

// Legacy interfaces
void* S_CALLTYPE SteamApps();
void* S_CALLTYPE SteamClient();
