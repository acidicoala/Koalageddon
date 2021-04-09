#include "pch.h"
#include "Logger.h"
#include "Config.h"

namespace Logger
{

path getLogsDirPath()
{
	return getWorkingDirPath() / "logs";
}

void init(string loggerName, bool truncate)
{
	Config::init(); // TODO: Remove
	if(config->log_level == "off")
		return;

	try
	{
		auto processName = getCurrentProcessPath().stem().string();
		auto fileName = fmt::format("{}.{}.log", loggerName, processName);
		auto path = getLogsDirPath() / fileName;

		logger = spdlog::basic_logger_mt(loggerName, path.wstring(), truncate);
		logger->set_pattern("[%H:%M:%S.%e] [%l]\t%v");
		logger->set_level(spdlog::level::from_str(config->log_level));
		logger->flush_on(spdlog::level::debug);
	} catch(const std::exception&)
	{
		// Now if we can't open log file, something must be really wrong, hence we exit.
		/*
		auto message = stow(string(ex.what()));
		MessageBox(NULL, message.c_str(), L"Failed to initialize the log file", MB_ICONERROR | MB_OK);
		exit(1);
		*/

		// EDIT:

		// Actually, it might be the case that multiple instances of the same process are launched
		// simultaneously. In this case it is very likely that they will fail at opening the file.
		// Example: QtWebEngineProcess.exe - issues ACCESS DENIED
	}
}

}

shared_ptr<spdlog::logger> logger = spdlog::null_logger_mt("null");
