// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "eos_common.h"

#pragma pack(push, 8)

EXTERN_C typedef struct EOS_EcomHandle* EOS_HEcom;

/**
 * This handle is copied when EOS_Ecom_CopyTransactionById or EOS_Ecom_CopyTransactionByIndex is called.
 * A EOS_Ecom_CheckoutCallbackInfo provides the ID for the copy.
 * After being copied, EOS_Ecom_Transaction_Release must be called.
 *
 * @see EOS_Ecom_CheckoutCallbackInfo
 * @see EOS_Ecom_CopyTransactionById
 * @see EOS_Ecom_CopyTransactionByIndex
 * @see EOS_Ecom_Transaction_Release
 */
EXTERN_C typedef struct EOS_Ecom_TransactionHandle* EOS_Ecom_HTransaction;

/**
 * A unique identifier for a catalog item defined and stored with the backend catalog service.
 * A catalog item represents a distinct object within the catalog.  When acquired by an account, an
 * entitlement is granted that references a specific catalog item.
 */
EXTERN_C typedef const char* EOS_Ecom_CatalogItemId;

/**
 * A unique identifier for a catalog offer defined and stored with the backend catalog service.
 * A catalog offer is a purchasable collection of 1 or more items, associated with a price (which
 * could be 0).  When an offer is purchased an entitlement is granted for each of the items
 * referenced by the offer.
 */
EXTERN_C typedef const char* EOS_Ecom_CatalogOfferId;

/**
 * An identifier which is defined on a catalog item and stored with the backend catalog service.
 * The entitlement name may not be unique.  A catalog may be configured with multiple items with the
 * same entitlement name in order to define a logical grouping of entitlements.  This is used to
 * retrieve all entitlements granted to an account grouped in this way.
 *
 * @see EOS_Ecom_QueryEntitlements
 */
EXTERN_C typedef const char* EOS_Ecom_EntitlementName;

/**
 * A unique identifier for an entitlement owned by an account.  An entitlement is always associated
 * with a single account.  The entitlement ID is provided to allow redeeming the entitlement as
 * well as identify individual entitlement grants.
 *
 * @see EOS_Ecom_QueryEntitlements
 * @see EOS_Ecom_RedeemEntitlements
 */
EXTERN_C typedef const char* EOS_Ecom_EntitlementId;


/**
 * An enumeration of the different ownership statuses.
 */
EOS_ENUM(EOS_EOwnershipStatus,
	/** The catalog item is not owned by the local user */
	EOS_OS_NotOwned = 0,
	/** The catalog item is owned by the local user */
	EOS_OS_Owned = 1
);

/**
 * An enumeration defining the type of catalog item.  The primary use is to identify how the item is expended.
 */
EOS_ENUM(EOS_EEcomItemType,
	/** This entitlement is intended to persist. */
	EOS_EIT_Durable = 0,
	/**
	 * This entitlement is intended to be transient and redeemed.
	 *
	 * @see EOS_Ecom_RedeemEntitlements
	 */
	EOS_EIT_Consumable = 1,
	/** This entitlement has a type that is not currently intneded for an in-game store. */
	EOS_EIT_Other = 2
);

/** The most recent version of the EOS_Ecom_Entitlement struct. */
#define EOS_ECOM_ENTITLEMENT_API_LATEST 2

/** Timestamp value representing an undefined EndTimestamp for EOS_Ecom_Entitlement */
#define EOS_ECOM_ENTITLEMENT_ENDTIMESTAMP_UNDEFINED -1

/**
 * Contains information about a single entitlement associated with an account. Instances of this structure are
 * created by EOS_Ecom_CopyEntitlementByIndex, EOS_Ecom_CopyEntitlementByNameAndIndex, or EOS_Ecom_CopyEntitlementById.
 * They must be passed to EOS_Ecom_Entitlement_Release.
 */
EOS_STRUCT(EOS_Ecom_Entitlement, (
	/** API Version: Set this to EOS_ECOM_ENTITLEMENT_API_LATEST. */
	int32_t ApiVersion;
	/** Name of the entitlement */
	EOS_Ecom_EntitlementName EntitlementName;
	/** ID of the entitlement owned by an account */
	EOS_Ecom_EntitlementId EntitlementId;
	/** ID of the item associated with the offer which granted this entitlement */
	EOS_Ecom_CatalogItemId CatalogItemId;
	/**
	 * If queried using pagination then ServerIndex represents the index of the entitlement as it
	 * exists on the server.  If not queried using pagination then ServerIndex will be -1.
	 */
	int32_t ServerIndex;
	/** If true then the catalog has this entitlement marked as redeemed */
	EOS_Bool bRedeemed;
	/** If not -1 then this is a POSIX timestamp that this entitlement will end */
	int64_t EndTimestamp;
));


/** The most recent version of the EOS_Ecom_ItemOwnership struct. */
#define EOS_ECOM_ITEMOWNERSHIP_API_LATEST 1

/**
 * Contains information about a single item ownership associated with an account. This structure is
 * returned as part of the EOS_Ecom_QueryOwnershipCallbackInfo structure.
 */
EOS_STRUCT(EOS_Ecom_ItemOwnership, (
	/** API Version: Set this to EOS_ECOM_ITEMOWNERSHIP_API_LATEST. */
	int32_t ApiVersion;
	/** ID of the catalog item */
	EOS_Ecom_CatalogItemId Id;
	/** Is this catalog item owned by the local user */
	EOS_EOwnershipStatus OwnershipStatus;
));

/** The most recent version of the EOS_Ecom_QueryOwnership API. */
#define EOS_ECOM_QUERYOWNERSHIP_API_LATEST 2

/**
 * The maximum number of catalog items that may be queried in a single pass
 */
#define EOS_ECOM_QUERYOWNERSHIP_MAX_CATALOG_IDS 32

/**
 * Input parameters for the EOS_Ecom_QueryOwnership function.
 */
EOS_STRUCT(EOS_Ecom_QueryOwnershipOptions, (
	/** API Version: Set this to EOS_ECOM_QUERYOWNERSHIP_API_LATEST. */
	int32_t ApiVersion;
	/** The Epic Online Services Account ID of the local user whose ownership to query */
	EOS_EpicAccountId LocalUserId;
	/** The array of Catalog Item IDs to check for ownership */
	EOS_Ecom_CatalogItemId* CatalogItemIds;
	/** The number of Catalog Item IDs to in the array */
	uint32_t CatalogItemIdCount;
	/** Optional product namespace, if not the one specified during initialization */
	const char* CatalogNamespace;
));

/**
 * Output parameters for the EOS_Ecom_QueryOwnership Function.
 */
EOS_STRUCT(EOS_Ecom_QueryOwnershipCallbackInfo, (
	/** The EOS_EResult code for the operation. EOS_Success indicates that the operation succeeded; other codes indicate errors. */
	EOS_EResult ResultCode;
	/** Context that was passed into EOS_Ecom_QueryOwnership */
	void* ClientData;
	/** The Epic Online Services Account ID of the local user whose ownership was queried */
	EOS_EpicAccountId LocalUserId;
	/** List of catalog items and their ownership status */
	const EOS_Ecom_ItemOwnership* ItemOwnership;
	/** Number of ownership results are included in this callback */
	uint32_t ItemOwnershipCount;
));

/**
 * Function prototype definition for callbacks passed to EOS_Ecom_QueryOwnership
 * @param Data A EOS_Ecom_QueryOwnershipCallbackInfo containing the output information and result
 */
EOS_DECLARE_CALLBACK(EOS_Ecom_OnQueryOwnershipCallback, const EOS_Ecom_QueryOwnershipCallbackInfo* Data);

/** The most recent version of the EOS_Ecom_QueryOwnershipToken API. */
#define EOS_ECOM_QUERYOWNERSHIPTOKEN_API_LATEST 2

/**
 * The maximum number of catalog items that may be queried in a single pass
 */
#define EOS_ECOM_QUERYOWNERSHIPTOKEN_MAX_CATALOGITEM_IDS 32

/**
 * Input parameters for the EOS_Ecom_QueryOwnershipToken function.
 */
EOS_STRUCT(EOS_Ecom_QueryOwnershipTokenOptions, (
	/** API Version: Set this to EOS_ECOM_QUERYOWNERSHIPTOKEN_API_LATEST. */
	int32_t ApiVersion;
	/** The Epic Online Services Account ID of the local user whose ownership token you want to query */
	EOS_EpicAccountId LocalUserId;
	/** The array of Catalog Item IDs to check for ownership, matching in number to the CatalogItemIdCount */
	EOS_Ecom_CatalogItemId* CatalogItemIds;
	/** The number of catalog item IDs to query */
	uint32_t CatalogItemIdCount;
	/** Optional product namespace, if not the one specified during initialization */
	const char* CatalogNamespace;
));

/**
 * Output parameters for the EOS_Ecom_QueryOwnershipToken Function.
 */
EOS_STRUCT(EOS_Ecom_QueryOwnershipTokenCallbackInfo, (
	/** The EOS_EResult code for the operation. EOS_Success indicates that the operation succeeded; other codes indicate errors. */
	EOS_EResult ResultCode;
	/** Context that was passed into EOS_Ecom_QueryOwnershipToken */
	void* ClientData;
	/** The Epic Online Services Account ID of the local user whose ownership token was queried */
	EOS_EpicAccountId LocalUserId;
	/** Ownership token containing details about the catalog items queried */
	const char* OwnershipToken;
));

/**
 * Function prototype definition for callbacks passed to EOS_Ecom_QueryOwnershipToken
 * @param Data A EOS_Ecom_QueryOwnershipTokenCallbackInfo containing the output information and result
 */
EOS_DECLARE_CALLBACK(EOS_Ecom_OnQueryOwnershipTokenCallback, const EOS_Ecom_QueryOwnershipTokenCallbackInfo* Data);

/** The most recent version of the EOS_Ecom_QueryEntitlements API. */
#define EOS_ECOM_QUERYENTITLEMENTS_API_LATEST 2

/**
 * The maximum number of entitlements that may be queried in a single pass
 */
#define EOS_ECOM_QUERYENTITLEMENTS_MAX_ENTITLEMENT_IDS 32

/**
 * Input parameters for the EOS_Ecom_QueryEntitlements function.
 */
EOS_STRUCT(EOS_Ecom_QueryEntitlementsOptions, (
	/** API Version: Set this to EOS_ECOM_QUERYENTITLEMENTS_API_LATEST. */
	int32_t ApiVersion;
	/** The Epic Online Services Account ID of the local user whose Entitlements you want to retrieve */
	EOS_EpicAccountId LocalUserId;
	/** An array of Entitlement Names that you want to check */
	EOS_Ecom_EntitlementName* EntitlementNames;
	/** The number of Entitlement Names included in the array, up to EOS_ECOM_QUERYENTITLEMENTS_MAX_ENTITLEMENT_IDS; use zero to request all Entitlements associated with the user's Epic Online Services account. */
	uint32_t EntitlementNameCount;
	/** If true, Entitlements that have been redeemed will be included in the results. */
	EOS_Bool bIncludeRedeemed;
));

/**
 * Output parameters for the EOS_Ecom_QueryEntitlements Function.
 */
EOS_STRUCT(EOS_Ecom_QueryEntitlementsCallbackInfo, (
	EOS_EResult ResultCode;
	/** Context that was passed into EOS_Ecom_QueryEntitlements */
	void* ClientData;
	/** The Epic Online Services Account ID of the local user whose entitlement was queried */
	EOS_EpicAccountId LocalUserId;
));

/**
 * Function prototype definition for callbacks passed to EOS_Ecom_QueryOwnershipToken
 * @param Data A EOS_Ecom_QueryEntitlementsCallbackInfo containing the output information and result
 */
EOS_DECLARE_CALLBACK(EOS_Ecom_OnQueryEntitlementsCallback, const EOS_Ecom_QueryEntitlementsCallbackInfo* Data);

/** The most recent version of the EOS_Ecom_GetEntitlementsCount API. */
#define EOS_ECOM_GETENTITLEMENTSCOUNT_API_LATEST 1

/**
 * Input parameters for the EOS_Ecom_GetEntitlementsCount function.
 */
EOS_STRUCT(EOS_Ecom_GetEntitlementsCountOptions, (
	/** API Version: Set this to EOS_ECOM_GETENTITLEMENTSCOUNT_API_LATEST. */
	int32_t ApiVersion;
	/** The Epic Online Services Account ID of the local user for which to retrieve the entitlement count */
	EOS_EpicAccountId LocalUserId;
));

/** The most recent version of the EOS_Ecom_GetEntitlementsByNameCount API. */
#define EOS_ECOM_GETENTITLEMENTSBYNAMECOUNT_API_LATEST 1

/**
 * Input parameters for the EOS_Ecom_GetEntitlementsByNameCount function.
 */
EOS_STRUCT(EOS_Ecom_GetEntitlementsByNameCountOptions, (
	/** API Version: Set this to EOS_ECOM_GETENTITLEMENTSBYNAMECOUNT_API_LATEST. */
	int32_t ApiVersion;
	/** The Epic Online Services Account ID of the local user for which to retrieve the entitlement count */
	EOS_EpicAccountId LocalUserId;
	/** Name of the entitlement to count in the cache */
	EOS_Ecom_EntitlementName EntitlementName;
));

/** The most recent version of the EOS_Ecom_CopyEntitlementByIndex API. */
#define EOS_ECOM_COPYENTITLEMENTBYINDEX_API_LATEST 1

/**
 * Input parameters for the EOS_Ecom_CopyEntitlementByIndex function.
 */
EOS_STRUCT(EOS_Ecom_CopyEntitlementByIndexOptions, (
	/** API Version: Set this to EOS_ECOM_COPYENTITLEMENTBYINDEX_API_LATEST. */
	int32_t ApiVersion;
	/** The Epic Online Services Account ID of the local user whose entitlement is being copied */
	EOS_EpicAccountId LocalUserId;
	/** Index of the entitlement to retrieve from the cache */
	uint32_t EntitlementIndex;
));

/** The most recent version of the EOS_Ecom_CopyEntitlementByNameAndIndex API. */
#define EOS_ECOM_COPYENTITLEMENTBYNAMEANDINDEX_API_LATEST 1

/**
 * Input parameters for the EOS_Ecom_CopyEntitlementByNameAndIndex function.
 */
EOS_STRUCT(EOS_Ecom_CopyEntitlementByNameAndIndexOptions, (
	/** API Version: Set this to EOS_ECOM_COPYENTITLEMENTBYNAMEANDINDEX_API_LATEST. */
	int32_t ApiVersion;
	/** The Epic Online Services Account ID of the local user whose entitlement is being copied */
	EOS_EpicAccountId LocalUserId;
	/** Name of the entitlement to retrieve from the cache */
	EOS_Ecom_EntitlementName EntitlementName;
	/** Index of the entitlement within the named set to retrieve from the cache. */
	uint32_t Index;
));

/** The most recent version of the EOS_Ecom_CopyEntitlementById API. */
#define EOS_ECOM_COPYENTITLEMENTBYID_API_LATEST 2

/**
 * Input parameters for the EOS_Ecom_CopyEntitlementById function.
 */
EOS_STRUCT(EOS_Ecom_CopyEntitlementByIdOptions, (
	/** API Version: Set this to EOS_ECOM_COPYENTITLEMENTBYID_API_LATEST. */
	int32_t ApiVersion;
	/** The Epic Online Services Account ID of the local user whose entitlement is being copied */
	EOS_EpicAccountId LocalUserId;
	/** ID of the entitlement to retrieve from the cache */
	EOS_Ecom_EntitlementId EntitlementId;
));

/** The most recent version of the EOS_Ecom_CopyItemById API. */
#define EOS_ECOM_COPYITEMBYID_API_LATEST 1

/**
 * Input parameters for the EOS_Ecom_CopyItemById function.
 */
EOS_STRUCT(EOS_Ecom_CopyItemByIdOptions, (
	/** API Version: Set this to EOS_ECOM_COPYITEMBYID_API_LATEST. */
	int32_t ApiVersion;
	/** The Epic Online Services Account ID of the local user whose item is being copied */
	EOS_EpicAccountId LocalUserId;
	/** The ID of the item to get. */
	EOS_Ecom_CatalogItemId ItemId;
));


/** The most recent version of the EOS_Ecom_GetItemReleaseCount API. */
#define EOS_ECOM_GETITEMRELEASECOUNT_API_LATEST 1

/**
 * Input parameters for the EOS_Ecom_GetItemReleaseCount function.
 */
EOS_STRUCT(EOS_Ecom_GetItemReleaseCountOptions, (
	/** API Version: Set this to EOS_ECOM_GETITEMRELEASECOUNT_API_LATEST. */
	int32_t ApiVersion;
	/** The Epic Online Services Account ID of the local user whose item release is being accessed */
	EOS_EpicAccountId LocalUserId;
	/** The ID of the item to get the releases for. */
	EOS_Ecom_CatalogItemId ItemId;
));

/** The most recent version of the EOS_Ecom_CopyItemReleaseByIndex API. */
#define EOS_ECOM_COPYITEMRELEASEBYINDEX_API_LATEST 1

/**
 * Input parameters for the EOS_Ecom_CopyItemReleaseByIndex function.
 */
EOS_STRUCT(EOS_Ecom_CopyItemReleaseByIndexOptions, (
	/** API Version: Set this to EOS_ECOM_COPYITEMRELEASEBYINDEX_API_LATEST. */
	int32_t ApiVersion;
	/** The Epic Online Services Account ID of the local user whose item release is being copied */
	EOS_EpicAccountId LocalUserId;
	/** The ID of the item to get the releases for. */
	EOS_Ecom_CatalogItemId ItemId;
	/** The index of the release to get. */
	uint32_t ReleaseIndex;
));

#pragma pack(pop)
