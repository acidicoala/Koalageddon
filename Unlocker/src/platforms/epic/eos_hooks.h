#pragma once
#include "eos_ecom_types.h"

/**
 * Query the ownership status for a given list of catalog item IDs defined with Epic Online Services.
 * This data will be cached for a limited time and retrieved again from the backend when necessary
 *
 * @param Options structure containing the account and catalog item IDs to retrieve
 * @param ClientData arbitrary data that is passed back to you in the CompletionDelegate
 * @param CompletionDelegate a callback that is fired when the async operation completes, either successfully or in error
 */
void EOS_CALL EOS_Ecom_QueryOwnership(
	EOS_HEcom Handle,
	const EOS_Ecom_QueryOwnershipOptions* Options,
	void* ClientData,
	const EOS_Ecom_OnQueryOwnershipCallback CompletionDelegate
);

/**
 * Query the entitlement information defined with Epic Online Services.
 * A set of entitlement names can be provided to filter the set of entitlements associated with the account.
 * This data will be cached for a limited time and retrieved again from the backend when necessary.
 * Use EOS_Ecom_CopyEntitlementByIndex, EOS_Ecom_CopyEntitlementByNameAndIndex, and EOS_Ecom_CopyEntitlementById to get the entitlement details.
 * Use EOS_Ecom_GetEntitlementsByNameCount to retrieve the number of entitlements with a specific entitlement name.
 *
 * @param Options structure containing the account and entitlement names to retrieve
 * @param ClientData arbitrary data that is passed back to you in the CompletionDelegate
 * @param CompletionDelegate a callback that is fired when the async operation completes, either successfully or in error
 */
void EOS_CALL EOS_Ecom_QueryEntitlements(
	EOS_HEcom Handle,
	const EOS_Ecom_QueryEntitlementsOptions* Options,
	void* ClientData,
	const EOS_Ecom_OnQueryEntitlementsCallback CompletionDelegate
);

/**
 * Fetch the number of entitlements that are cached for a given local user.
 *
 * @param Options structure containing the Epic Online Services Account ID being accessed
 *
 * @see EOS_Ecom_CopyEntitlementByIndex
 *
 * @return the number of entitlements found.
 */
uint32_t EOS_CALL EOS_Ecom_GetEntitlementsCount(
	EOS_HEcom Handle,
	const EOS_Ecom_GetEntitlementsCountOptions* Options
);

/**
 * Fetches an entitlement from a given index.
 *
 * @param Options structure containing the Epic Online Services Account ID and index being accessed
 * @param OutEntitlement the entitlement for the given index, if it exists and is valid, use EOS_Ecom_Entitlement_Release when finished
 *
 * @see EOS_Ecom_Entitlement_Release
 *
 * @return EOS_Success if the information is available and passed out in OutEntitlement
 *         EOS_Ecom_EntitlementStale if the entitlement information is stale and passed out in OutEntitlement
 *         EOS_InvalidParameters if you pass a null pointer for the out parameter
 *         EOS_NotFound if the entitlement is not found
 */
EOS_EResult EOS_CALL EOS_Ecom_CopyEntitlementByIndex(
	EOS_HEcom Handle,
	const EOS_Ecom_CopyEntitlementByIndexOptions* Options,
	EOS_Ecom_Entitlement** OutEntitlement
);

/**
 * Release the memory associated with an EOS_Ecom_Entitlement structure. This must be called on data
 * retrieved from EOS_Ecom_CopyEntitlementByIndex and EOS_Ecom_CopyEntitlementById.
 *
 * @param Entitlement - The entitlement structure to be released
 *
 * @see EOS_Ecom_Entitlement
 * @see EOS_Ecom_CopyEntitlementByIndex
 * @see EOS_Ecom_CopyEntitlementById
 */
void EOS_CALL EOS_Ecom_Entitlement_Release(
	EOS_Ecom_Entitlement* Entitlement
);
