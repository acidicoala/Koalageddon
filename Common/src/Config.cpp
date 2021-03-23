#include "pch.h"
#include "Config.h"
#include "util.h"

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

void from_json(const json& j, SteamPlatform& p)
{
	from_json(j, (Platform&) p);
	j["unlock_shared_library"].get_to(p.unlock_shared_library);
}

void from_json(const json& j, Platforms& p)
{
	j["Steam"].get_to(p.Steam);
	j["Epic Games"].get_to(p.EpicGames);
	j["Origin"].get_to(p.Origin);
	j["Uplay R1"].get_to(p.UplayR1);
}

Config::Config()
{
	auto fullPath = getWorkingDirPath() / CONFIG_NAME;

	std::ifstream ifs(fullPath, std::ios::in);

	if(!ifs.good())
	{
		MessageBox(NULL, fullPath.c_str(), L"Config not found at: ", MB_ICONERROR);
		exit(1);
	}

	try
	{
		auto j = json::parse(ifs, nullptr, true, true);

		GET(j, log_level);
		GET(j, platforms);
		GET(j, ignore);
		GET(j, terminate);

		platformRefs = j["platforms"].get<Platforms>();
	} catch(json::exception e)
	{
		MessageBoxA(NULL, e.what(), "Error parsing config file", MB_ICONERROR);
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
