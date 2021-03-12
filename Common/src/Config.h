#pragma once
#include "framework.h"
#include "util.h"

struct Platform
{
	bool enabled = true;
	string process;
	bool replicate = false;
	vector<string> ignore;
	vector<string> blacklist;
};

class Config
{
protected:
	Config();
public:
	string log_level;
	map<string, Platform> platforms;
	vector<string> ignore;
	vector<string> terminate;

	static void init();
};

extern Config* config;
