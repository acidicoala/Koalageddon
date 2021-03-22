#include "pch.h"
#include "util.h"
#include "Logger.h"
#include "constants.h"
#include <iterator> // ???


bool contains(wstring haystack, wstring needle)
{
	return haystack.find(needle) != wstring::npos;
}

bool startsWith(string word, string prefix)
{
	return word.find(prefix, 0) == 0;
}

bool endsWith(string word, string postfix)
{
	return word.find(postfix, 0) == word.length() - postfix.length();
}

string toLower(string str)
{
	string lowerString = str; // Copy constructor.
	std::for_each(lowerString.begin(), lowerString.end(), [](char& c){
		c = ::tolower(c);
	});
	return lowerString;
}

/// Case-insensitive string comparison. Returns true if strings are equal.
bool stringsAreEqual(string one, string two)
{
	return toLower(one) == toLower(two);
}

wstring getProcessName(DWORD pid)
{
	auto defaultName = wstring(L"<unknown process>");
	auto hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);
	if(hProcess == NULL)
	{
		logger->error("Failed to open handle to a process with id: {}. Error code: 0x{0:}", pid, GetLastError());
		return defaultName;
	}

	TCHAR buffer[MAX_PATH];
	auto result = GetModuleFileNameEx(hProcess, NULL, buffer, MAX_PATH);
	if(result == NULL)
	{
		logger->error("Failed to get process name with id: {}s. Error code: 0x{0:}", pid, GetLastError());
		return defaultName;
	}

	return wstring(buffer);
}

path getCurrentProcessPath()
{
	TCHAR buffer[MAX_PATH];

	if(GetModuleFileName(NULL, buffer, MAX_PATH) == NULL)
		logger->error("Failed to get current process file name. Error code: {}", GetLastError());

	return absolute(buffer);
}

string getCurrentProcessName()
{
	return getCurrentProcessPath().filename().string();
}

/**
Returns std::filesystem::path of the process identified by the
provided handle. If handle is not provided, then currently running
process path is returned.
*/
path getProcessPath(HANDLE handle)
{
	TCHAR buffer[MAX_PATH];

	if(GetModuleFileNameEx(handle, NULL, buffer, MAX_PATH) == NULL)
	{
		auto message = fmt::format("Failed to obtain process path. Error code: 0x{:X}", GetLastError());
		throw std::exception(message.c_str());
	}

	return absolute(buffer);
}

path getWorkingDirPath()
{
	return absolute(getReg(WORKING_DIR));
}

bool is32bit(DWORD PID)
{
	BOOL isWow64;

	auto process = OpenProcess(PROCESS_QUERY_INFORMATION, TRUE, PID);
	if(process == NULL)
	{ // Should not happen often. If it does, worst case is failed injection.
		logger->error("Failed to get a handle to process with PID: {}", PID);
		return true;
	}

	IsWow64Process(process, &isWow64);
	CloseHandle(process);

	// If a process is running under WOW64, then it means it is 32-bit.
	return isWow64;
}

bool is32bit(HANDLE hProcess)
{
	// Cannot use IsWow64Process2 since it is supported by Win 10 only
	BOOL isWow64 = NULL;
	IsWow64Process(hProcess, &isWow64);
	return isWow64;
}

void killProcess(HANDLE hProcess, DWORD sleepMS)
{
	TerminateProcess(hProcess, 0);
	auto result = WaitForSingleObject(hProcess, 3 * 1000); // 3s should be enough
	if(result != WAIT_OBJECT_0)
	{
		auto processName = getProcessPath(hProcess).string();
		throw std::exception(fmt::format("Failed to terminate process: {}", processName).c_str());
	}
	else
	{
		Sleep(sleepMS); // Some other process may still be locking the file
	}
}


// Source: https://stackoverflow.com/a/3999597/3805929

// Convert a wide Unicode string to a UTF8 string
std::string wtos(const std::wstring& wstr)
{
	if(wstr.empty()) return std::string();
	int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int) wstr.size(), NULL, 0, NULL, NULL);
	std::string strTo(size_needed, 0);
	WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int) wstr.size(), &strTo[0], size_needed, NULL, NULL);
	return strTo;
}

// Convert a UTF8 string to a wide Unicode String
std::wstring stow(const std::string& str)
{
	if(str.empty()) return std::wstring();
	int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int) str.size(), NULL, 0);
	std::wstring wstrTo(size_needed, 0);
	MultiByteToWideChar(CP_UTF8, 0, &str[0], (int) str.size(), &wstrTo[0], size_needed);
	return wstrTo;
}


char* makeCStringCopy(string src)
{
	const auto len = src.length();
	char* dest = new char[len + 1];
	std::size_t length = src.copy(dest, len, 0);
	dest[len] = '\0';
	return dest;
}

wstring getReg(LPCWSTR key)
{
	static winreg::RegKey regKey{ HKEY_CURRENT_USER, L"SOFTWARE\\acidicoala\\Koalageddon" };
	return regKey.GetStringValue(key);
}

void setReg(LPCWSTR key, LPCWSTR val)
{
	static winreg::RegKey regKey{ HKEY_CURRENT_USER, L"SOFTWARE\\acidicoala\\Koalageddon" };
	regKey.SetStringValue(key, val);
}
string readFileContents(string path)
{
	std::ifstream fileStream(path);
	if(fileStream.good())
		return string(std::istreambuf_iterator<char>{fileStream}, {});
	else
		return "";
}

bool writeFileContents(path filePath, string contents)
{
	if(!std::filesystem::exists(filePath))
		std::filesystem::create_directories(filePath.parent_path());

	std::ofstream fileStream(filePath);
	if(fileStream.good())
	{
		fileStream << contents;
		return true;
	}
	else
	{
		logger->error("Failed to write to file: {}", filePath.string());
		return false;
	}
}

void showFatalError(string message, bool terminate)
{
	message = fmt::format("{}\nLast error: 0x{:X}", message, GetLastError());
	logger->error(message);
	MessageBoxA(NULL, message.c_str(), "Fatal Error", MB_ICONERROR | MB_OK);

	if(terminate)
		exit(1);
}

void showInfo(string message, string title, bool shouldLog)
{
	if(shouldLog)
		logger->info(message);

	MessageBoxA(NULL, message.c_str(), title.c_str(), MB_ICONINFORMATION | MB_OK);
}

