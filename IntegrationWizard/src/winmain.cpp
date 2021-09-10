#include "pch.h"
#include "Logger.h"
#include "Config.h"
#include "constants.h"
#include "IntegrationWizard.h"
#include "integration_wizard_util.h"
#include "../resource.h"

void firstSetup() {
	setReg(KOALAGEDDON_KEY, INSTALL_DIR, getCurrentProcessPath().parent_path().wstring());
	setReg(KOALAGEDDON_KEY, WORKING_DIR, (getProgramDataPath() / ACIDICOALA / KOALAGEDDON).wstring());

	// Load the default config into memory
	HRSRC hResource = FindResource(nullptr, MAKEINTRESOURCE(IDR_DEFAULT_CONFIG), L"CONFIG");
	if (hResource == nullptr) {
		fatalError("Failed to find config resource");
		return;
	}

	HGLOBAL hMemory = LoadResource(nullptr, hResource);
	if (hMemory == nullptr) {
		fatalError("Failed to load config resource");
		return;
	}

	auto dataSize = SizeofResource(nullptr, hResource);
	auto dataPtr = LockResource(hMemory);

	if (exists(getConfigPath())) {
		// If config exists, check if it needs updating
		auto ifs = std::ifstream(getConfigPath(), std::ios::in);
		if (!ifs.good()) {
			fatalError("Failed to open input stream to read existing config file");
			return;
		}

		auto configJson = json::parse(ifs, nullptr, true, true);
		auto configVersion = configJson["config_version"].get<int>();

		auto is = std::istringstream(string(static_cast<char*>(dataPtr), dataSize));
		auto defaultConfigJson = json::parse(is, nullptr, true, true);
		auto defaultConfigVersion = defaultConfigJson["config_version"].get<int>();

		// Do not copy if the log versions are matching
		if (configVersion == defaultConfigVersion) {
			return;
		}
	}

	// Copy the default config
	create_directories(getConfigPath().parent_path());
	std::ofstream configFile(getConfigPath(), std::ios::out | std::ios::binary);
	if (!configFile.good()) {
		fatalError("Failed to open output file stream when writing config file");
		return;
	}

	configFile.write(static_cast<char*>(dataPtr), dataSize);
	configFile.close();
}

void askForAction(
	const HINSTANCE hInstance,
	map<int, IntegrationWizard::PlatformInstallation>& platforms,
	Action* action,
	int* platformID,
	BOOL* createShortcut
) {
	TASKDIALOGCONFIG tdc = {};
	tdc.cbSize = sizeof(TASKDIALOGCONFIG);
	const auto szTitle = stow(fmt::format("Koalageddon Wizard v{}", VERSION));
	constexpr auto szHeader = L"Welcome to the Koalageddon wizard";
	constexpr auto szBodyText = L"Please select the platform for which you wish to install/remove integrations";

	constexpr auto szExpandedInformation =
		L"The wizard will scan windows registry to find target platforms. "
		L"During installation/removal platform processes will be terminated, "
		L"so make sure to close all games and save all data.";

	vector<TASKDIALOG_BUTTON> radioButtons;
	radioButtons.reserve(platforms.size() + 1);
	for (const auto& [key, platform] : platforms) {
		radioButtons.push_back(TASKDIALOG_BUTTON{key, platform.name.c_str()});
	}

	if (radioButtons.size() > 1) {
		radioButtons.push_back(TASKDIALOG_BUTTON{IntegrationWizard::ALL_PLATFORMS, L"All of the above"});
	}

	TASKDIALOG_BUTTON aCustomButtons[] = {
		{static_cast<int>(Action::INSTALL_INTEGRATIONS), L"&Install platform integrations"},
		{static_cast<int>(Action::REMOVE_INTEGRATIONS), L"&Remove platform integrations"}
	};

	const auto wFooter = fmt::format(
		LR"(📂 <a href="{0}">Open config directory</a>  ({0})

			🌐 <a href="{1}">Open latest release page</a>  ({1})

			💬 <a href="{2}">Open support forum topic</a>  ({2}))",
		getConfigPath().parent_path().wstring(),
		L"https://github.com/acidicoala/Koalageddon/releases/latest",
		L"https://cs.rin.ru/forum/viewtopic.php?p=2333491#p2333491"
	);

	tdc.hInstance = hInstance;
	tdc.dwFlags = TDF_ALLOW_DIALOG_CANCELLATION | TDF_USE_COMMAND_LINKS | TDF_EXPAND_FOOTER_AREA | TDF_ENABLE_HYPERLINKS;
	tdc.pButtons = aCustomButtons;
	tdc.cButtons = _countof(aCustomButtons);
	tdc.pszWindowTitle = szTitle.c_str();
	tdc.pszMainIcon = TD_INFORMATION_ICON;
	tdc.pszMainInstruction = szHeader;
	tdc.cRadioButtons = radioButtons.size();
	tdc.nDefaultRadioButton = IntegrationWizard::ALL_PLATFORMS;
	tdc.pRadioButtons = radioButtons.data();
	tdc.pszContent = szBodyText;
	tdc.pszExpandedInformation = szExpandedInformation;
	tdc.pszVerificationText = L"Create desktop shortcut to configuration file";
	tdc.pszFooter = wFooter.c_str();
	// tdc.pszFooterIcon = TD_INFORMATION_ICON;
	// ReSharper disable once CppParameterNeverUsed
	tdc.pfCallback = [](HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, LONG_PTR lpRefData)-> HRESULT {
		if (msg == TDN_HYPERLINK_CLICKED) {
			ShellExecute(
				nullptr,
				L"open",
				reinterpret_cast<LPCWSTR>(lParam), // NOLINT(performance-no-int-to-ptr)
				nullptr,
				nullptr,
				SW_SHOWNORMAL
			);
		}

		return S_OK;
	};

	if (SUCCEEDED(TaskDialogIndirect(&tdc, reinterpret_cast<int*>(action), platformID, createShortcut))) {
		logger->debug("Clicked button: {}", *action);

		if (*action != Action::INSTALL_INTEGRATIONS && *action != Action::REMOVE_INTEGRATIONS)
			*action = Action::NO_ACTION;
	} else {
		*action = Action::UNEXPECTED_ERROR;
	}
}

int APIENTRY wWinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR lpCmdLine,
	_In_ int nCmdShow
) {
	firstSetup();
	Config::init();
	Logger::init("IntegrationWizard", true);
	logger->info("Integration Wizard v{}", VERSION);

	// Files created by the Integration Wizard will belong to Administrators.
	// Therefore, we need to change their ownership to Users.
	changeOwnership(getWorkingDirPath().parent_path().c_str(), WinBuiltinUsersSid);

	// We need to disable WOW64 redirections because otherwise 32bit application
	// uses SysWOW64 directory even if System32 was explicitly provided.

	auto platforms = IntegrationWizard::getInstalledPlatforms();

	Action action;
	int platformID = -1;
	BOOL shouldCreateShortcut = FALSE;

	if (platforms.empty())
		action = Action::NOTHING_TO_INSTALL;
	else
		askForAction(hInstance, platforms, &action, &platformID, &shouldCreateShortcut);

	switch (action) {
		case Action::UNEXPECTED_ERROR:
			logger->error("Unexpected action result. Error code: 0x{:X}", GetLastError());
			break;
		case Action::NO_ACTION:
			logger->info("No action was taken. Terminating.");
			break;
		case Action::INSTALL_INTEGRATIONS:
			[[fallthrough]];
		case Action::REMOVE_INTEGRATIONS:
			alterPlatform(action, platformID, platforms);
			if (shouldCreateShortcut) {
				createShortcut(
					getConfigPath().wstring(),
					(getDesktopPath() / "Config.lnk").wstring(),
					L"Koalageddon Configuration File"
				);
			}
			break;
		case Action::NOTHING_TO_INSTALL:
			MessageBox(nullptr, L"Koalageddon did not find any installed platforms.", L"Nothing found", MB_ICONINFORMATION);
			break;
	}

	return 0;
}
