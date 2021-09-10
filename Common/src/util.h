#pragma once
#include "framework.h"

// Import into global namespace commonly used classes
using std::string;
using std::wstring;
using std::vector;
using std::pair;
using std::map;
using std::shared_ptr;
using std::unique_ptr;
using std::make_unique;
using std::filesystem::absolute;
using std::filesystem::path;
using std::filesystem::copy_options;
using nlohmann::json;

constexpr auto INJECTOR_64 = L"Injector64.exe";
constexpr auto INJECTOR_32 = L"Injector32.exe";

constexpr auto UNLOCKER_64 = L"Unlocker64.dll";
constexpr auto UNLOCKER_32 = L"Unlocker32.dll";


#ifdef _WIN64
constexpr auto UNLOCKER_NAME = "Unlocker64";
constexpr auto UNLOCKER_DLL = "Unlocker64.dll";
#else
constexpr auto UNLOCKER_NAME = "Unlocker32";
constexpr auto UNLOCKER_DLL = "Unlocker32.dll";
#endif


// Process info
wstring getProcessName(DWORD pid);
path getCurrentProcessPath();
string getCurrentProcessName();
path getProcessPath(HANDLE handle);
HANDLE getProcessHandle(string name, DWORD dwAccess = PROCESS_ALL_ACCESS);
bool is32bit(DWORD PID);
bool is32bit(HANDLE hProcess);
void killProcess(HANDLE hProcess, DWORD sleepMS = 0);
void killProcess(string name);
string getModuleVersion(string filename);
MODULEINFO getModuleInfo(HMODULE hModule);

// String utils
string wtos(const wstring& wstr);
wstring stow(const string& wstr);
char* makeCStringCopy(string src);
bool contains(string haystack, string needle, bool insensitive = true);
bool contains(wstring haystack, wstring needle);
bool startsWith(string word, string prefix);
bool endsWith(string word, string postfix);
string toLower(string str);
bool stringsAreEqual(string one, string two, bool insensitive = true);

// Registry
wstring getReg(string key, string valueName);
void setReg(string key, string valueName, wstring data);
path getWorkingDirPath();
path getInstallDirPath();
path getCacheDirPath();
path getConfigPath();

// File access
string readFileContents(path path);
bool writeFileContents(path filePath, string contents);

// Message Box helpers
void showFatalError(string message, bool terminate, bool showLastError = true);
void showInfo(string message, string title = "Information", bool shouldLog = false);

template<typename T>
bool vectorContains(vector<T> elements, T element)
{
	return std::find(elements.begin(), elements.end(), element) != elements.end();
}

// Requests
template <typename... Ts>
cpr::Response fetch(string url, Ts&&... ts)
{
	return cpr::Get(
		cpr::Url{ url },
		cpr::Timeout{ 1 * 1000 }, // 1s
		cpr::VerifySsl{ false },
		std::forward<Ts>(ts)...
	);
}
