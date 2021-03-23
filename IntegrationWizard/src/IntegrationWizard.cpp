#include "pch.h"
#include "IntegrationWizard.h"
#include "Logger.h"
#include "Config.h"
#include "constants.h"

vector<string> alteredPlatforms;


/*
Returns the path to original version.dll based on the architecture
of the provided process handle.
*/
path getVersionDllPath(HANDLE hProcess)
{
	PWSTR rawPath;
	auto folderID = is32bit(hProcess) ? FOLDERID_SystemX86 : FOLDERID_System;
	SHGetKnownFolderPath(folderID, NULL, NULL, &rawPath);
	auto systemPath = absolute(rawPath);
	CoTaskMemFree(rawPath);
	return systemPath / "version.dll";
}

// Source: https://docs.microsoft.com/en-us/windows/win32/psapi/enumerating-all-processes
void enumerateProcesses(function<void(HANDLE hProcess, path processPath, string processName)> callback)
{
	DWORD aProcesses[1024], cbNeeded, cProcesses;

	if(!EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded))
		showFatalError("Failed to enumerate processes.", true);

	// Calculate how many process identifiers were returned.
	cProcesses = cbNeeded / sizeof(DWORD);

	logger->debug("Found running processes:");

	// Print the name and process identifier for each process.
	for(DWORD i = 0; i < cProcesses; i++)
	{
		auto pid = aProcesses[i];
		if(pid == 0)
			continue;

		// Get a handle to the process.
		HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);

		if(hProcess == NULL)
			continue; // Not a big deal, just skip it.

		try
		{
			auto processPath = getProcessPath(hProcess);
			auto processName = processPath.filename().string();
			logger->debug("\tName: '{}', pid: {}", processName, pid);

			for(const auto& [key, platform] : config->platforms)
			{
				if(stringsAreEqual(processName, platform.process))
				{
					logger->info("Target process detected: '{}'", processName);
					callback(hProcess, processPath, processName);
				}
			}
		} catch(std::exception& ex)
		{
			logger->warn("Error in handling a process with PID: {}. Reason: {}", pid, ex.what());
		}


		CloseHandle(hProcess);
	}
}

void installCallback(HANDLE hProcess, path processPath, string processName)
{
	try
	{
		logger->info("Installing platform integration");

		auto originalDllPath = getVersionDllPath(hProcess);
		auto originalVersionDllPath = processPath.parent_path() / "version_o.dll";
		logger->debug("Original DLL path: '{}', Destination: '{}'", originalDllPath.string(), originalVersionDllPath.string());

		auto integrationDllPath = getWorkingDirPath() / (is32bit(hProcess) ? INTEGRATION_32 : INTEGRATION_64);
		auto versionDLLPath = processPath.parent_path() / "version.dll";
		logger->debug("Integration DLL path: '{}', Destination: '{}'", integrationDllPath.string(), versionDLLPath.string());

		// Terminate the process to release a possible lock on the files
		killProcess(hProcess);

		copy_file(originalDllPath, originalVersionDllPath, copy_options::overwrite_existing);
		copy_file(integrationDllPath, versionDLLPath, copy_options::overwrite_existing);
		
		alteredPlatforms.push_back(processName);
		logger->info("Platform integration was successfully installed");

	} catch(std::exception& ex)
	{
		showFatalError(fmt::format("Failed to install integrations: {}", ex.what()), false);
	}
}

void removeCallback(HANDLE hProcess, path processPath, string processName)
{
	try
	{
		logger->info("Removing platform integration");

		auto versionDLLPath = stow((processPath.parent_path() / "version.dll").string());
		auto originalDllPath = stow((processPath.parent_path() / "version_o.dll").string());
		logger->debug(L"Version DLL path: '{}', Original DLL path: '{}'", versionDLLPath, originalDllPath);

		// Terminate the process to release a lock on the files
		killProcess(hProcess, 250);

		DeleteFile(versionDLLPath.c_str());
		DeleteFile(originalDllPath.c_str());

		alteredPlatforms.push_back(processName);
		logger->info("Platform integration was successfully removed");

	} catch(std::exception& ex)
	{
		showFatalError(fmt::format("Failed to remove integrations: {}", ex.what()), false);
	}
}

void showPostActionReport(Action action)
{
	if(alteredPlatforms.size() == 0)
	{
		showInfo("No target processes were found. No actions were taken.", "Nothing found", true);
	}
	else
	{
		string targets;
		for(const auto& target : alteredPlatforms)
		{
			auto symbol = action == Action::INSTALL_INTEGRATIONS ? "+" : "-";
			targets += fmt::format("[{}] {}\n", symbol, target);
		}

		auto actionStr = action == Action::INSTALL_INTEGRATIONS ? "installed" : "removed";
		auto message = fmt::format("The following target platforms integrations were sucessfully {}:\n\n{}", actionStr, targets);
		showInfo(message, "Success");
	}
}

void IntegrationWizard::install()
{
	logger->info("Installing integrations");

	enumerateProcesses(installCallback);

	showPostActionReport(Action::INSTALL_INTEGRATIONS);
}

void IntegrationWizard::remove()
{
	logger->info("Removing integrations");

	enumerateProcesses(removeCallback);

	showPostActionReport(Action::REMOVE_INTEGRATIONS);
}
