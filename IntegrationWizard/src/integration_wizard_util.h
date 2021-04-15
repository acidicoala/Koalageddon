#pragma once
#include "util.h"

bool changeOwnership(LPCWSTR objectPath, WELL_KNOWN_SID_TYPE wellKnownSidType);
void fatalError(string message);
wstring getEnvVar(wstring key);
path getProgramDataPath();
path getDesktopPath();
HRESULT createShortcut(wstring targetLocation, wstring shortcutLocation, wstring description);
