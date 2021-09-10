#pragma once

constexpr auto VERSION = "1.5.4";

constexpr auto INTEGRATION_64 = L"Integration64.dll";
constexpr auto INTEGRATION_32 = L"Integration32.dll";

#ifdef _WIN64
constexpr auto INTEGRATION = INTEGRATION_64;

constexpr auto EOSSDK = L"EOSSDK-Win64-Shipping.dll";
constexpr auto STEAMAPI = L"steam_api64.dll";
constexpr auto STEAM_CLIENT = L"steamclient64.dll";
constexpr auto UPLAY_R1 = L"uplay_r1_loader64.dll";
constexpr auto UPLAY_R2 = L"uplay_r2_loader64.dll";

#else
constexpr auto INTEGRATION = INTEGRATION_32;

constexpr auto EOSSDK = L"EOSSDK-Win32-Shipping.dll";
constexpr auto STEAMAPI = L"steam_api.dll";
constexpr auto STEAM_CLIENT = L"steamclient.dll";
constexpr auto UPLAY_R1 = L"uplay_r1_loader.dll";
constexpr auto UPLAY_R2 = L"uplay_r2_loader.dll";

#endif

constexpr auto ORIGINCLIENT = L"OriginClient.dll"; // x86 only
constexpr auto EA_DESKTOP = L"Qt5Core.dll"; // x86-64 only


constexpr auto origin_entitlements_url = "https://raw.githubusercontent.com/acidicoala/public-entitlements/main/origin/v1/entitlements.json";
constexpr auto steamclient_patterns_url = "https://raw.githubusercontent.com/acidicoala/public-entitlements/main/koalageddon/v1/steamclient-patterns.json";
constexpr auto latest_release_url_api = "https://api.github.com/repos/acidicoala/Koalageddon/releases/latest";
constexpr auto latest_release_url = "https://github.com/acidicoala/Koalageddon/releases/latest";

// Registry & paths
constexpr auto KOALAGEDDON_KEY = R"(SOFTWARE\acidicoala\Koalageddon)";
constexpr auto WORKING_DIR = "WORKING_DIR";
constexpr auto INSTALL_DIR = "INSTALL_DIR";
constexpr auto CONFIG_NAME = "Config.jsonc";
constexpr auto ACIDICOALA = "acidicoala";
constexpr auto KOALAGEDDON = "Koalageddon";

// Platform info

constexpr auto STEAM_NAME = "Steam";
constexpr auto STEAM_PROCESS = "Steam.exe";
constexpr auto STEAM_KEY = R"(SOFTWARE\Valve\Steam)";
constexpr auto STEAM_VALUE = "InstallPath";

constexpr auto EPIC_GAMES_32_NAME = "Epic Games (x32)";
constexpr auto EPIC_GAMES_64_NAME = "Epic Games (x64)";
constexpr auto EPIC_GAMES_PROCESS = "EpicGamesLauncher.exe";
constexpr auto EPIC_GAMES_KEY = R"(SOFTWARE\EpicGames\Unreal Engine)";
constexpr auto EPIC_GAMES_VALUE = "INSTALLDIR";

constexpr auto UBISOFT_NAME = "Ubisoft";
constexpr auto UBISOFT_PROCESS = "Upc.exe";
constexpr auto UBISOFT_KEY = R"(SOFTWARE\Ubisoft\Launcher)";
constexpr auto UBISOFT_VALUE = "InstallDir";

constexpr auto ORIGIN_NAME = "Origin";
constexpr auto ORIGIN_PROCESS = "Origin.exe";
constexpr auto ORIGIN_KEY = R"(SOFTWARE\Origin)";
constexpr auto ORIGIN_VALUE = "OriginPath";

constexpr auto EA_DESKTOP_NAME = "EA Desktop";
constexpr auto EA_DESKTOP_PROCESS = "EADesktop.exe";
constexpr auto EA_DESKTOP_KEY = R"(SOFTWARE\Electronic Arts\EA Desktop)";
constexpr auto EA_DESKTOP_VALUE = "DesktopAppPath";

constexpr auto EPIC_GAMES_NAME = "Epic Games";
constexpr auto STEAM_CLIENT_NAME = "Steam Client";
