#pragma once
#include "util.h"

struct Platform
{
	bool enabled = true;
	string process;
	bool replicate = false;
	vector<string> ignore;
	vector<string> blacklist;
};

struct SteamPlatform : Platform
{
	bool unlock_dlc = true;
	bool unlock_shared_library = false;
	vector<string> app_blacklist;
};

struct Platforms
{
	SteamPlatform Steam;
	Platform EpicGames;
	Platform Origin;
	Platform EADesktop;
	Platform UplayR1;
};

class Config
{
protected:
	Config();
public:
	string log_level;
	Platforms platformRefs;
	map<string, Platform> platforms;
	vector<string> ignore;
	vector<string> terminate;

	static void init();
};

extern Config* config;
