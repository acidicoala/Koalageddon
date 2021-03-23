#include "pch.h"
#include "Logger.h"
#include "Config.h"
#include "constants.h"
#include "IntegrationWizard.h"
#include "../resource.h"

Action askForAction(HINSTANCE hInstance)
{
	TASKDIALOGCONFIG tdc = { sizeof(TASKDIALOGCONFIG) };
	int nClickedBtn;
	auto szTitle = L"Koalageddon";
	auto szHeader = L"Welcome to the Koalageddon wizard. Please choose the desired action.";
	LPCWSTR szBodyText =
		L"The wizard will scan running processes to find target platforms. " \
		L"During installation/removal target processes will be terminated, " \
		L"so make sure to close all games and save all data.";

	TASKDIALOG_BUTTON aCustomButtons[] = {
	  { (int) Action::INSTALL_INTEGRATIONS, L"&Install platform integrations" },
	  { (int) Action::REMOVE_INTEGRATIONS, L"&Remove platform integrations" }
	};

	tdc.hInstance = hInstance;
	tdc.dwFlags = TDF_ALLOW_DIALOG_CANCELLATION | TDF_USE_COMMAND_LINKS | TDF_EXPAND_FOOTER_AREA;
	tdc.pButtons = aCustomButtons;
	tdc.cButtons = _countof(aCustomButtons);
	tdc.pszWindowTitle = szTitle;
	tdc.pszMainIcon = TD_INFORMATION_ICON;
	tdc.pszMainInstruction = szHeader;
	//tdc.pszContent = szBodyText;
	tdc.pszExpandedInformation = szBodyText;

	if(SUCCEEDED(TaskDialogIndirect(&tdc, &nClickedBtn, NULL, NULL)))
	{
		logger->debug("Clicked button: {}", nClickedBtn);

		if(nClickedBtn == (int) Action::INSTALL_INTEGRATIONS ||
			nClickedBtn == (int) Action::REMOVE_INTEGRATIONS)
			return (Action) nClickedBtn;
		else
			return Action::NO_ACTION;
	}
	else
	{
		return Action::UNEXPECTED_ERROR;
	}

}

void fatalError(string message)
{
	message = fmt::format("{}. Error code: {}", message, (void*) GetLastError());
	MessageBoxA(NULL, message.c_str(), "Fatal Error", MB_ICONERROR | MB_OK);
	exit(1);
}

void firstSetup()
{
	setReg(WORKING_DIR, getCurrentProcessPath().parent_path().c_str());

	auto configPath = getWorkingDirPath() / CONFIG_NAME;

	// Load the default config into memory
	HRSRC hResource = FindResource(nullptr, MAKEINTRESOURCE(IDR_DEFAULT_CONFIG), L"CONFIG");
	if(hResource == NULL)
	{
		fatalError("Failed to find config resource");
		return;
	}

	HGLOBAL hMemory = LoadResource(nullptr, hResource);
	if(hMemory == NULL)
	{
		fatalError("Failed to load config resource");
		return;
	}

	auto size = SizeofResource(nullptr, hResource);
	auto dataPtr = LockResource(hMemory);

	if(std::filesystem::exists(configPath))
	{ // If config exists, check if it needs updating
		auto ifs = std::ifstream(getWorkingDirPath() / CONFIG_NAME, std::ios::in);
		if(!ifs.good())
		{
			fatalError("Failed to open input stream to read existing config file");
			return;
		}

		auto configJson = nlohmann::json::parse(ifs, nullptr, true, true);
		auto logLevel = configJson["config_version"].get<int>();

		auto is = std::istringstream(string((char*) dataPtr));
		auto defaultConfigJson = nlohmann::json::parse(is, nullptr, true, true);
		auto defaultLogLevel = defaultConfigJson["config_version"].get<int>();

		// Do not copy if the log versions are matching
		if(logLevel == defaultLogLevel)
			return;
	}

	// Copy the default config
	std::ofstream configFile(configPath, std::ios::out | std::ios::binary);
	if(!configFile.good())
	{
		fatalError("Failed to open output file stream when writing config file");
		return;
	}

	configFile.write((char*) dataPtr, size);
	configFile.close();


}


int APIENTRY wWinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR lpCmdLine,
	_In_ int nCmdShow
)
{
	firstSetup();
	Config::init();
	Logger::init("IntegrationWizard", true);
	logger->info("Integration Wizard v{}", VERSION);

	// We need to disable WOW64 redirections because otherwise 32bit application
	// uses SysWOW64 directory even if System32 was explicitly provided.
	void* oldVal = NULL;
	Wow64DisableWow64FsRedirection(&oldVal);

	auto action = askForAction(hInstance);

	switch(action)
	{
		case Action::UNEXPECTED_ERROR:
			logger->error("Unexpected action result. Error code: 0x{:X}", GetLastError());
			break;
		case Action::NO_ACTION:
			logger->info("No action was taken. Terminating.");
			break;
		case Action::INSTALL_INTEGRATIONS:
			IntegrationWizard::install();
			break;
		case Action::REMOVE_INTEGRATIONS:
			IntegrationWizard::remove();
			break;
		default:
			logger->error("Unexpected action result");
	}

	Wow64RevertWow64FsRedirection(oldVal);
	return 0;
}

