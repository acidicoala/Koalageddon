#pragma once

#include "platforms/BasePlatform.h"
#include "util.h"

#ifdef _WIN64
constexpr auto STR_EOS_Ecom_QueryOwnership = "EOS_Ecom_QueryOwnership";
constexpr auto STR_EOS_Ecom_QueryEntitlements = "EOS_Ecom_QueryEntitlements";
constexpr auto STR_EOS_Ecom_GetEntitlementsCount = "EOS_Ecom_GetEntitlementsCount";
constexpr auto STR_EOS_Ecom_CopyEntitlementByIndex = "EOS_Ecom_CopyEntitlementByIndex";
constexpr auto STR_EOS_Ecom_Entitlement_Release = "EOS_Ecom_Entitlement_Release";
#else
constexpr auto STR_EOS_Ecom_QueryOwnership = "_EOS_Ecom_QueryOwnership@16";
constexpr auto STR_EOS_Ecom_QueryEntitlements = "_EOS_Ecom_QueryEntitlements@16";
constexpr auto STR_EOS_Ecom_GetEntitlementsCount = "_EOS_Ecom_GetEntitlementsCount@8";
constexpr auto STR_EOS_Ecom_CopyEntitlementByIndex = "_EOS_Ecom_CopyEntitlementByIndex@12";
constexpr auto STR_EOS_Ecom_Entitlement_Release = "_EOS_Ecom_Entitlement_Release@4";
#endif

class Epic : public BasePlatform
{
protected:
	void platformInit() override;
	string getPlatformName() override;
	LPCWSTR getModuleName() override;
	Hooks& getPlatformHooks() override;
public:
	using BasePlatform::BasePlatform;

	inline static Hooks hooks;
};
