#pragma once
#include "util.h"

// IMPORTANT: Since this files directly defines members,
// it can only be included once.

const auto STEAM_APPS = string("STEAMAPPS_INTERFACE_VERSION");
const auto STEAM_CLIENT = string("SteamClient");
const auto STEAM_USER = string("SteamUser");

typedef map<string, uint16_t> VFuncOrdinalMap;
typedef map<string, VFuncOrdinalMap> InterfaceMap;

// https://github.com/SteamRE/open-steamworks/tree/master/Steam4NET2/Steam4NET2/autogen
// Maps interfaces to their function ordinals
InterfaceMap ordinalMap = {
	{STEAM_APPS, { // This is a rather stable interface
		{"BIsSubscribedApp", 6}, // [002 - 008]. Missing in [001]
		{"BIsDlcInstalled", 7}, // [003 - 008]. Missing in [001 - 002]
		{"GetDLCCount", 10}, // [004 - 008]. Missing in [001 - 003]
		{"BGetDLCDataByIndex", 11} // [004 - 008]. Missing in [001 - 003]
	}},
	{STEAM_CLIENT, { // TODO: Patche the ordinal dynamically?
		/*
		001: N/A.
		...
		006: 16
		007: 18
		008: 15
		009: 16
		...
		011: 16
		012: 15
		...
		020: 15
		*/
		{"GetISteamApps", 15} // Missing in [001 - 005]
	}},
	{STEAM_USER, { // Do we really need it anyway?
		/*
		001: N/A
		...
		012: 15
		013: 16
		...
		015: 17
		...
		021: 17
		*/
		{"UserHasLicenseForApp", 17} // Missing in [001 - 011].
	}}
};
