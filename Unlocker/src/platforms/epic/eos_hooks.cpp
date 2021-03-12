#include "pch.h"
#include "eos_hooks.h"
#include "Epic.h"

#define GET_PROXY_FUNC(FUNC) \
	static auto proxyFunc = PLH::FnCast(BasePlatform::trampolineMap[#FUNC], FUNC);

static vector<string> entitlements;

auto getEpicConfig()
{
	return config->platforms["Epic Games"];
}

void EOS_CALL EOS_Ecom_QueryOwnership(
	EOS_HEcom Handle,
	const EOS_Ecom_QueryOwnershipOptions* Options,
	void* ClientData,
	const EOS_Ecom_OnQueryOwnershipCallback CompletionDelegate
)
{
	auto itemCount = Options->CatalogItemIdCount;
	auto ownerships = new EOS_Ecom_ItemOwnership[itemCount];

	logger->info("Game requested ownership of {} items", itemCount);
	for(uint32_t i = 0; i < itemCount; i++)
	{
		// Epic magic happens here
		ownerships[i].ApiVersion = EOS_ECOM_ITEMOWNERSHIP_API_LATEST;
		ownerships[i].Id = Options->CatalogItemIds[i];
		auto isBlacklisted = vectorContains(getEpicConfig().blacklist, string(ownerships[i].Id));
		ownerships[i].OwnershipStatus = isBlacklisted ? EOS_EOwnershipStatus::EOS_OS_NotOwned : EOS_EOwnershipStatus::EOS_OS_Owned;

		logger->info("\t{} [{}]", ownerships[i].Id, (bool) ownerships[i].OwnershipStatus ? "Owned" : "NotOwned");
	}

	EOS_Ecom_QueryOwnershipCallbackInfo callbackInfo =
	{
		EOS_EResult::EOS_Success,
		ClientData,
		Options->LocalUserId,
		ownerships,
		Options->CatalogItemIdCount
	};

	CompletionDelegate(&callbackInfo);

	delete[] ownerships;
}

void EOS_CALL EOS_Ecom_QueryEntitlements(
	EOS_HEcom Handle,
	const EOS_Ecom_QueryEntitlementsOptions* Options,
	void* ClientData,
	const EOS_Ecom_OnQueryEntitlementsCallback CompletionDelegate
)
{
	auto entitlementCount = Options->EntitlementNameCount;

	logger->info("Game requested ownership of {} entitlements", entitlementCount);

	entitlements.clear();
	for(uint32_t i = 0; i < entitlementCount; i++)
	{
		auto isBlacklisted = vectorContains(getEpicConfig().blacklist, string(Options->EntitlementNames[i]));
		if(!isBlacklisted)
		{
			// Save the entitlements id for response in subsequent queries
			entitlements.push_back(Options->EntitlementNames[i]);
		}
	}

	EOS_Ecom_QueryEntitlementsCallbackInfo callbackData = {
		EOS_EResult::EOS_Success,
		ClientData,
		Options->LocalUserId
	};

	CompletionDelegate(&callbackData);
}

uint32_t EOS_CALL EOS_Ecom_GetEntitlementsCount(
	EOS_HEcom Handle,
	const EOS_Ecom_GetEntitlementsCountOptions* Options
)
{
	logger->debug("Game requested count of user entitlements");

	auto entitlementCount = (uint32_t) entitlements.size();
	if(entitlementCount == 0)
	{
		logger->warn("No entitlements were queried. Redirecting to original function.");
		GET_PROXY_FUNC(EOS_Ecom_GetEntitlementsCount);
		entitlementCount = proxyFunc(Handle, Options);
	}

	logger->info("Responding with {} entitlements", entitlementCount);

	return entitlementCount;
}

EOS_EResult EOS_CALL EOS_Ecom_CopyEntitlementByIndex(
	EOS_HEcom Handle,
	const EOS_Ecom_CopyEntitlementByIndexOptions* Options,
	EOS_Ecom_Entitlement** OutEntitlement
)
{
	logger->debug("EOS_Ecom_CopyEntitlementByIndex -> ApiVersion: {}", Options->ApiVersion);
	logger->debug("Game requested entitlement with index: {}", Options->EntitlementIndex);

	if(Options->EntitlementIndex >= entitlements.size())
	{
		logger->warn("Out of bounds entitlement index. Redirecting to original function.");
		GET_PROXY_FUNC(EOS_Ecom_CopyEntitlementByIndex);
		return proxyFunc(Handle, Options, OutEntitlement);
	}
	else
	{
		// Epic magic happens here (2)

		const char* id = makeCStringCopy(entitlements.at(Options->EntitlementIndex));

		logger->info("\t{}", id);

		*OutEntitlement = new EOS_Ecom_Entitlement{
			EOS_ECOM_ENTITLEMENT_API_LATEST,
			id, // EntitlementName
			id, // EntitlementId
			id, // CatalogItemId
			-1, // ServerIndex
			false, // bRedeemed
			-1 // EndTimestamp
		};

		return EOS_EResult::EOS_Success;
	}
}

void EOS_CALL EOS_Ecom_Entitlement_Release(EOS_Ecom_Entitlement* Entitlement)
{
	logger->debug("Game requested to release entitlement: {}", Entitlement->EntitlementId);

	delete[] Entitlement->EntitlementId;
	delete Entitlement;
}
