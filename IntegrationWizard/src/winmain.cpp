#include "pch.h"
#include "Logger.h"
#include "Config.h"
#include "constants.h"
#include "IntegrationWizard.h"
#include "../resource.h"

void fatalError(string message)
{
	message = fmt::format("{}. Error code: 0x{:X}", message, GetLastError());
	MessageBoxA(NULL, message.c_str(), "Fatal Error", MB_ICONERROR | MB_OK);
	exit(1);
}

void firstSetup()
{
	setReg(KOALAGEDDON_KEY, WORKING_DIR, getCurrentProcessPath().parent_path().string());

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


void askForAction(
	HINSTANCE hInstance,
	map<int, IntegrationWizard::PlatformInstallation>& platforms,
	Action* action,
	int* platformID
)
{
	TASKDIALOGCONFIG tdc = { sizeof(TASKDIALOGCONFIG) };
	auto szTitle = L"Koalageddon Wizard";
	auto szHeader = L"Welcome to the Koalageddon wizard.";
	auto szBodyText =
		L"Please select the platform for which you wish to install/remove integrations";

	LPCWSTR szExpandedInformation =
		L"The wizard will scan windows registry to find target platforms. " \
		L"During installation/removal platform processes will be terminated, " \
		L"so make sure to close all games and save all data.";

	vector<TASKDIALOG_BUTTON> radioButtons;
	for(const auto& [key, platform] : platforms)
		radioButtons.push_back(TASKDIALOG_BUTTON{ key, platform.name.c_str() });

	if(radioButtons.size() > 1)
		radioButtons.push_back(TASKDIALOG_BUTTON{ IntegrationWizard::ALL_PLATFORMS, L"All of the above" });

	TASKDIALOG_BUTTON aCustomButtons[] = {
	  { (int) Action::INSTALL_INTEGRATIONS, L"&Install platform integrations" },
	  { (int) Action::REMOVE_INTEGRATIONS, L"&Remove platform integrations" }
	};

	LPCWSTR szFooter =
		LR"(<a href="https://github.com/acidicoala/Koalageddon/releases">Download latest release</a>)";

	tdc.hInstance = hInstance;
	tdc.dwFlags = TDF_ALLOW_DIALOG_CANCELLATION | TDF_USE_COMMAND_LINKS | TDF_EXPAND_FOOTER_AREA | TDF_ENABLE_HYPERLINKS;
	tdc.pButtons = aCustomButtons;
	tdc.cButtons = _countof(aCustomButtons);
	tdc.pszWindowTitle = szTitle;
	tdc.pszMainIcon = TD_INFORMATION_ICON;
	tdc.pszMainInstruction = szHeader;
	tdc.cRadioButtons = (UINT) radioButtons.size();
	tdc.nDefaultRadioButton = IntegrationWizard::ALL_PLATFORMS;
	tdc.pRadioButtons = radioButtons.data();
	tdc.pszContent = szBodyText;
	tdc.pszExpandedInformation = szExpandedInformation;
	tdc.pszFooter = szFooter;
	tdc.pszFooterIcon = TD_INFORMATION_ICON;

	if(SUCCEEDED(TaskDialogIndirect(&tdc, (int*) action, platformID, NULL)))
	{
		logger->debug("Clicked button: {}", *action);

		if(*action != Action::INSTALL_INTEGRATIONS && *action != Action::REMOVE_INTEGRATIONS)
			*action = Action::NO_ACTION;
	}
	else
	{
		*action = Action::UNEXPECTED_ERROR;
	}
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

	auto platforms = IntegrationWizard::getInstalledPlatforms();

	Action action;
	int platformID;

	if(platforms.empty())
		action = Action::NOTHING_TO_INSTALL;
	else
		askForAction(hInstance, platforms, &action, &platformID);

	switch(action)
	{
		case Action::UNEXPECTED_ERROR:
			logger->error("Unexpected action result. Error code: 0x{:X}", GetLastError());
			break;
		case Action::NO_ACTION:
			logger->info("No action was taken. Terminating.");
			break;
		case Action::INSTALL_INTEGRATIONS:
		case Action::REMOVE_INTEGRATIONS:
			IntegrationWizard::alterPlatform(action, platformID, platforms);
			break;
		case Action::NOTHING_TO_INSTALL:
			MessageBox(NULL, L"Koalageddon did not find any installed platforms.", L"Nothing found", MB_ICONINFORMATION);
			break;
		default:
			fatalError("Unexpected action result");
	}

	Wow64RevertWow64FsRedirection(oldVal);
	return 0;
}

