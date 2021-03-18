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

constexpr auto INJECTOR_64 = L"Injector64.exe";
constexpr auto INJECTOR_32 = L"Injector32.exe";

constexpr auto UNLOCKER_64 = L"Unlocker64.dll";
constexpr auto UNLOCKER_32 = L"Unlocker32.dll";

#ifdef _WIN64
constexpr auto UNLOCKER_NAME = "Unlocker64";
#else
constexpr auto UNLOCKER_NAME = "Unlocker32";
#endif

// Process info
wstring getProcessName(DWORD pid);
wstring getCurrentProcessName();
path getProcessPath(HANDLE handle = NULL);
bool is32bit(DWORD PID);
bool is32bit(HANDLE hProcess);
void killProcess(HANDLE hProcess, DWORD sleepMS = 0);

// String utils
string wtos(const wstring& wstr);
wstring stow(const string& wstr);
char* makeCStringCopy(string src);
bool contains(wstring haystack, wstring needle);
bool startsWith(string word, string prefix);
bool endsWith(string word, string postfix);
string toLower(string str);
bool stringsAreEqual(string one, string two);

// Registry
wstring getReg(LPCWSTR key);
void setReg(LPCWSTR key, LPCWSTR val);
path getWorkingDirPath();

// File access
string readFileContents(string path);
bool writeFileContents(path filePath, string contents);

// Message Box helpers
void showFatalError(string message, bool terminate);
void showInfo(string message, string title = "Information", bool shouldLog = false);


template<typename T>
bool vectorContains(vector<T> elements, T element)
{
	return std::find(elements.begin(), elements.end(), element) != elements.end();
}
