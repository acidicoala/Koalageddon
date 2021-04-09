#include "pch.h"
#include "util.h"
#include "constants.h"
#include "Injector.h"
#include "Logger.h"
#include "Config.h"

// Hide console window & use wmain instead of main
#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:wmainCRTStartup")
int wmain(int argc, wchar_t* argv[], wchar_t* envp[])
{
	Config::init();
	Logger::init("Injector", false);

	logger->debug("Injector v{}", VERSION);

	if(argc != 3)
	{
		logger->error("Expected 2 arguments, received: {}", argc - 1);
		return ERROR_INVALID_ARGUMENTS;
	}

	try
	{
		auto PID = (DWORD) std::stoi(argv[1]);
		auto dllPath = absolute(argv[2]);

		logger->info(L"Injecting DLL into '{}'", getProcessName(PID));
		logger->debug(L"PID: {}, dllPath: '{}'", PID, dllPath.wstring());

		auto result = injectDLL(PID, dllPath);

		if(result == OK)
		{
			logger->info("DLL was successully injected");
		}
		else
		{
			logger->error("Failed to inject the DLL. Error code: 0x{:X}", result);
		}

		return result;
	} catch(std::logic_error&)
	{
		logger->error(L"Failed to convert PID {} to int", argv[1]);
		return ERROR_INVALID_ARGUMENTS;
	}
}
