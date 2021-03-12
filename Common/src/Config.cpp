#include "pch.h"
#include "Config.h"
#include "util.h"

using nlohmann::json;

// Source: https://stackoverflow.com/a/54394658/3805929
#define GET(j, key) this->key = j[#key].get<decltype(key)>()

void from_json(const json& j, Platform& p)
{
	j["enabled"].get_to(p.enabled);
	j["process"].get_to(p.process);
	j["replicate"].get_to(p.replicate);
	j["ignore"].get_to(p.ignore);
	j["blacklist"].get_to(p.blacklist);
}

Config::Config()
{
	auto fullPath = getWorkingDirPath() / L"Config.jsonc";

	std::ifstream ifs(fullPath, std::ios::in);

	if(!ifs.good())
	{
		MessageBox(NULL, fullPath.c_str(), L"Config not found at: ", MB_ICONERROR | MB_ICONERROR);
		exit(1);
	}

	try
	{
		auto j = json::parse(ifs, nullptr, true, true);

		GET(j, log_level);
		GET(j, platforms);
		GET(j, ignore);
		GET(j, terminate);
	} catch(json::exception e)
	{
		MessageBoxA(NULL, e.what(), "Error parsing config file", MB_ICONERROR | MB_ICONERROR);
		exit(1);
	}
}

void Config::init()
{
	if(config != nullptr)
		return;

	config = new Config();
}

// Every app must call the config constructor first.
Config* config = nullptr;
