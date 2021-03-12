#pragma once

constexpr auto VERSION = "1.0.0";
constexpr auto WORKING_DIR = L"WORKING_DIR";

constexpr auto INTEGRATION_64 = L"Integration64.dll";
constexpr auto INTEGRATION_32 = L"Integration32.dll";

#ifdef _WIN64

constexpr auto EOSSDK = L"EOSSDK-Win64-Shipping.dll";
constexpr auto STEAMAPI = L"steam_api64.dll";
constexpr auto UPLAY_R2 = L"uplay_r2_loader64.dll";

#else

constexpr auto EOSSDK = L"EOSSDK-Win32-Shipping.dll";
constexpr auto STEAMAPI = L"steam_api.dll";
constexpr auto UPLAY_R2 = L"uplay_r2_loader.dll";

#endif

constexpr auto ORIGINCLIENT = L"OriginClient.dll";
