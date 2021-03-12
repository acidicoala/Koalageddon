// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "eos_base.h"

#pragma pack(push, 8)

#undef EOS_RESULT_VALUE
#undef EOS_RESULT_VALUE_LAST
#define EOS_RESULT_VALUE(Name, Value) Name = Value,
#define EOS_RESULT_VALUE_LAST(Name, Value) Name = Value

EOS_ENUM_START(EOS_EResult)
#include "eos_result.h"
EOS_ENUM_END(EOS_EResult);

#undef EOS_RESULT_VALUE
#undef EOS_RESULT_VALUE_LAST

/**
 * A handle to a user's Epic Online Services Account ID
 * This ID is associated with a specific login associated with Epic Account Services
 *
 * @see EOS_Auth_Login
 */
typedef struct EOS_EpicAccountIdDetails* EOS_EpicAccountId;

/** 
 * A handle to a user's Product User ID (game services related ecosystem)
 * This ID is associated with any of the external account providers (of which Epic Account Services is one)
 * 
 * @see EOS_Connect_Login
 * @see EOS_EExternalCredentialType 
 */
typedef struct EOS_ProductUserIdDetails* EOS_ProductUserId;


#pragma pack(pop)
