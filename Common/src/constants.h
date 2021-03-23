#pragma once

constexpr auto VERSION = "1.2.0";
constexpr auto WORKING_DIR = L"WORKING_DIR";

constexpr auto INTEGRATION_64 = L"Integration64.dll";
constexpr auto INTEGRATION_32 = L"Integration32.dll";

#ifdef _WIN64

constexpr auto EOSSDK = L"EOSSDK-Win64-Shipping.dll";
constexpr auto STEAMAPI = L"steam_api64.dll";
constexpr auto STEAM_CLIENT = L"steamclient64.dll";
constexpr auto UPLAY_R1 = L"uplay_r1_loader64.dll";
constexpr auto UPLAY_R2 = L"uplay_r2_loader64.dll";

#else

constexpr auto EOSSDK = L"EOSSDK-Win32-Shipping.dll";
constexpr auto STEAMAPI = L"steam_api.dll";
constexpr auto STEAM_CLIENT = L"steamclient.dll";
constexpr auto UPLAY_R1 = L"uplay_r1_loader.dll";
constexpr auto UPLAY_R2 = L"uplay_r2_loader.dll";

#endif

constexpr auto ORIGINCLIENT = L"OriginClient.dll";


constexpr auto origin_entitlements_url = "https://raw.githubusercontent.com/acidicoala/public-entitlements/main/origin/v1/entitlements.json";
constexpr auto steamclient_offsets_url = "https://raw.githubusercontent.com/acidicoala/public-entitlements/main/koalageddon/v1/steamclient.json";
