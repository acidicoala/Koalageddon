#include "pch.h"
#include "util.h"
#include "constants.h"
#include "Injector.h"
#include "Logger.h"
#include "Config.h"

// Hide console window
#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")

int main(int argc, char** argv)
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
		auto dllPath = stow(argv[2]);

		logger->info(L"Injecting DLL into \"{}\"", getProcessName(PID));
		logger->debug(L"PID: {}, dllPath: \"{}\"", PID, dllPath);

		auto result = injectDLL(PID, dllPath);

		if(result == OK)
		{
			logger->info("DLL was successully injected");
		}
		else
		{
			logger->error("Failed to inject the DLL. Error code: 0x{0:}", result);
		}

		return result;
	} catch(std::logic_error&)
	{
		logger->error("Failed to convert PID {} to int", argv[1]);
		return ERROR_INVALID_ARGUMENTS;
	}
}
