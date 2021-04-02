#include "pch.h"
#include "IntegrationWizard.h"
#include "Logger.h"
#include "Config.h"

using namespace IntegrationWizard;

vector<wstring> IntegrationWizard::alteredPlatforms;

path getVersionDllPath(Architecture architecture)
{
	PWSTR rawPath;
	auto folderID = architecture == Architecture::x32 ? FOLDERID_SystemX86 : FOLDERID_System;
	SHGetKnownFolderPath(folderID, NULL, NULL, &rawPath);
	auto systemPath = absolute(rawPath);
	CoTaskMemFree(rawPath);
	return systemPath / "version.dll";
}

void showPostActionReport(Action action)
{
	auto actionedString = action == Action::INSTALL_INTEGRATIONS ? "installed" : "removed";

	if(alteredPlatforms.size() == 0)
	{
		auto message = fmt::format("No platforms integrations were {}", actionedString);
		showInfo(message, "No result", true);
	}
	else
	{
		string targets;
		for(const auto& target : alteredPlatforms)
		{
			auto symbol = action == Action::INSTALL_INTEGRATIONS ? "+" : "-";
			targets += fmt::format("[{}] {}\n", symbol, wtos(target));
		}

		auto actionStr = action == Action::INSTALL_INTEGRATIONS ? "installed" : "removed";
		auto message = fmt::format(
			"The following platform integrations were sucessfully {}:\n\n{}",
			actionStr, targets
		);
		showInfo(message, "Success");
	}
}

void installIntegration(const PlatformInstallation& platform)
{
	auto originalDllPath = getVersionDllPath(platform.architecture);
	auto originalVersionDllPath = platform.path / "version_o.dll";
	logger->debug(
		"Original DLL path: '{}', Destination: '{}'",
		originalDllPath.string(), originalVersionDllPath.string()
	);

	auto integrationDLL = platform.architecture == Architecture::x32 ? INTEGRATION_32 : INTEGRATION_64;
	auto integrationDllPath = getInstallDirPath() / integrationDLL;
	auto versionDLLPath = platform.path / "version.dll";
	logger->debug(
		"Integration DLL path: '{}', Destination: '{}'",
		integrationDllPath.string(), versionDLLPath.string()
	);

	// Terminate the process to release a possible lock on the files
	killProcess(platform.process);

	copy_file(originalDllPath, originalVersionDllPath, copy_options::overwrite_existing);
	copy_file(integrationDllPath, versionDLLPath, copy_options::overwrite_existing);
}

void removeIntegration(const PlatformInstallation& platform)
{
	auto versionDLLPath = platform.path / "version.dll";
	auto originalDllPath = platform.path / "version_o.dll";
	logger->debug(
		"Version DLL path: '{}', Original DLL path: '{}'",
		versionDLLPath.string(), originalDllPath.string()
	);

	// Terminate the process to release potential locks on files
	killProcess(platform.process);

	DeleteFile(versionDLLPath.c_str());
	DeleteFile(originalDllPath.c_str());
}


void safelyAlterPlatform(Action action, const PlatformInstallation& platform)
{
	auto actionString = action == Action::INSTALL_INTEGRATIONS ? "install" : "remove";
	auto actioningString = action == Action::INSTALL_INTEGRATIONS ? "Installing" : "Removing";
	auto actionedString = action == Action::INSTALL_INTEGRATIONS ? "installed" : "removed";

	auto callback = action == Action::INSTALL_INTEGRATIONS ? installIntegration : removeIntegration;

	try
	{
		logger->info("{} '{}' platform integration", actioningString, wtos(platform.name));
		callback(platform);

		alteredPlatforms.push_back(platform.name);
		logger->info("Platform integration was successfully {}", actionedString);
	} catch(std::exception& ex)
	{
		showFatalError(fmt::format(
			"Failed to {} {} integrations: {}",
			actionString, wtos(platform.name), ex.what()
		), false);
	}
}

void IntegrationWizard::alterPlatform(Action action, int platformID, map<int, PlatformInstallation> platforms)
{
	auto actioningString = action == Action::INSTALL_INTEGRATIONS ? "Installing" : "Removing";

	logger->info("{} integrations", actioningString);

	if(platformID == IntegrationWizard::ALL_PLATFORMS)
	{
		for(const auto& [key, platform] : platforms)
		{
			safelyAlterPlatform(action, platform);
		}
	}
	else
	{
		safelyAlterPlatform(action, platforms[platformID]);
	}

	showPostActionReport(action);
}

map<string, PlatformRegistry> platformRegMap = {
	// TODO: Use config process instead of hardcoded
	{EA_DESKTOP_NAME,	PlatformRegistry{ Architecture::x64, EA_DESKTOP_PROCESS,EA_DESKTOP_KEY,	EA_DESKTOP_VALUE}},
	{EPIC_GAMES_32_NAME,PlatformRegistry{ Architecture::x32, EPIC_GAMES_PROCESS,EPIC_GAMES_KEY,	EPIC_GAMES_VALUE}},
	{EPIC_GAMES_64_NAME,PlatformRegistry{ Architecture::x64, EPIC_GAMES_PROCESS,EPIC_GAMES_KEY,	EPIC_GAMES_VALUE}},
	{ORIGIN_NAME,		PlatformRegistry{ Architecture::x32, ORIGIN_PROCESS,	ORIGIN_KEY,		ORIGIN_VALUE	}},
	{STEAM_NAME,		PlatformRegistry{ Architecture::x32, STEAM_PROCESS,		STEAM_KEY,		STEAM_VALUE		}},
	{UBISOFT_NAME,		PlatformRegistry{ Architecture::x32, UBISOFT_PROCESS,	UBISOFT_KEY,	UBISOFT_VALUE	}},
};

map<int, PlatformInstallation> IntegrationWizard::getInstalledPlatforms()
{
	map<int, PlatformInstallation> installedPlatforms;

	int platformID = 1000;
	for(const auto& [name, platformRegistry] : platformRegMap)
	{
		const auto& [architecture, process, key, value] = platformRegistry;

		try
		{
			auto platformPath = absolute(getReg(key, value));

			// Epic binaries are located in sub-directories
			if(name == EPIC_GAMES_32_NAME)
				platformPath /= R"(Launcher\Portal\Binaries\Win32)";
			else if(name == EPIC_GAMES_64_NAME)
				platformPath /= R"(Launcher\Portal\Binaries\Win64)";
			else if(name == ORIGIN_NAME || name==EA_DESKTOP_NAME) // Origin & EA Desktop store path to exe
				platformPath = platformPath.parent_path();

			if(std::filesystem::exists(platformPath))
				installedPlatforms[platformID++] = PlatformInstallation{ platformPath, architecture, process, stow(name) };
		} catch(winreg::RegException& e)
		{ // This is normal if platform is not installed
			logger->warn(
				"Failed to obtain platform path from registry. Key: {}, Value: {}, Message: {}, Code: {}",
				key, value, e.what(), e.code().value()
			);
		}
	}

	return installedPlatforms;
}

