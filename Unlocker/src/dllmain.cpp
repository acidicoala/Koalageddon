#include "pch.h"
#include "Unlocker.h"

BOOL WINAPI DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	switch(ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
			Unlocker::init(hModule);
			break;
		case DLL_PROCESS_DETACH:
			Unlocker::shutdown();
			break;
	}

	return TRUE;
}
