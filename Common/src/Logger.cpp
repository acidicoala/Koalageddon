#include "pch.h"
#include "Logger.h"
#include "Config.h"

namespace Logger
{

void init(string loggerName, bool truncate)
{
	Config::init();
	if(config->log_level == "off")
		return;

	try
	{
		auto fileName = fmt::format("{}.{}.log", loggerName, getCurrentProcessName());
		auto path = getWorkingDirPath() / "logs" / fileName;
		logger = spdlog::basic_logger_mt(loggerName, path.u8string(), truncate);
		logger->set_pattern("[%H:%M:%S.%e] [%l]\t%v");
		logger->set_level(spdlog::level::from_str(config->log_level));
		logger->flush_on(spdlog::level::debug);
	} catch(const spdlog::spdlog_ex& ex)
	{
		// Now if we can't open log file, something must be really wrong, hence we exit.
		auto message = stow(string(ex.what()));
		MessageBox(NULL, message.c_str(), L"Failed to initialize the log file", MB_ICONERROR | MB_OK);
		exit(1);
	}

}

}

shared_ptr<spdlog::logger> logger = spdlog::null_logger_mt("null");
