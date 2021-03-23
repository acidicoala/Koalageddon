#pragma once
#include "util.h"
#include "constants.h"

enum class Action
{
	NO_ACTION = 1000,
	UNEXPECTED_ERROR = 1001,
	INSTALL_INTEGRATIONS = 1002,
	REMOVE_INTEGRATIONS = 1003,
	NOTHING_TO_INSTALL = 1004,
};

namespace IntegrationWizard
{

constexpr auto ALL_PLATFORMS = -1;
extern vector<wstring> alteredPlatforms;

enum class Architecture { x32, x64 };

struct PlatformInstallation
{
	path path;
	Architecture architecture = Architecture::x32;
	string process;
	wstring name;
};

struct PlatformRegistry
{
	Architecture architecture;
	string process;
	string key;
	string value;
};

map<int, PlatformInstallation> getInstalledPlatforms();
void alterPlatform(Action action, int platformID, map<int, PlatformInstallation> platforms);

}
