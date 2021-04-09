#pragma once
#include <codeanalysis\warnings.h>

#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers
#include <Windows.h>
#include <Psapi.h>
#include <tlhelp32.h>

#include <string>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <map>
#include <vector>
#include <memory>
#include <regex>
#include <thread>
#include <chrono>

// Following definitions are required for static build of libcurl
#pragma comment(lib,"Ws2_32.lib")
#pragma comment(lib,"Wldap32.lib")
#pragma comment(lib,"Crypt32.lib")

#pragma warning(push) // Disable 3rd party library warnings
#pragma warning(disable: ALL_CODE_ANALYSIS_WARNINGS)

#define SPDLOG_WCHAR_TO_UTF8_SUPPORT
#define SPDLOG_WCHAR_FILENAMES
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/null_sink.h>
#include <nlohmann/json.hpp>
#include <cpr/cpr.h> 
#include <winreg/WinReg.hpp>
#include <polyhook2/Detour/ADetour.hpp>
#include <polyhook2/IHook.hpp>
#include <tinyxml2.h>
#pragma warning(pop)
