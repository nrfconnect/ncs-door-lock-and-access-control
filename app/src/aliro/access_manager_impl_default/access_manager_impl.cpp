/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "access_manager_impl.h"
#include "access_document.h"
#include "aliro/aliro.h"
#include "aliro/mutex_guard.h"
#include "aliro/time.h"
#include "aliro/utils.h"
#include "crypto/crypto.h"
#include "storage.h"
#include "storage_keys.h"

#ifdef CONFIG_DOOR_LOCK_STEP_UP_PHASE
#include "cbor/access_document_decode.h"
#endif // CONFIG_DOOR_LOCK_STEP_UP_PHASE

#ifdef CONFIG_DOOR_LOCK_BLE_UWB
#include "aliro/memory.h"
#include "uwb_impl.h"
#endif // CONFIG_DOOR_LOCK_BLE_UWB

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/byteorder.h>

#include <algorithm>
#include <cstddef>
#include <cstdint>

LOG_MODULE_REGISTER(access_manager, CONFIG_DOOR_LOCK_APP_LOG_LEVEL);

namespace Aliro {

namespace {

constexpr ValidityIteration kMaxValidityIterationDiff{ 8 };

K_MUTEX_DEFINE(sMutex);

#ifdef CONFIG_DOOR_LOCK_STEP_UP_PHASE

#ifdef CONFIG_CHIP

constexpr std::array<uint8_t, 7> kElementIdentifier{ 'm', 'a', 't', 't', 'e', 'r', '1' };

#else // CONFIG_CHIP

constexpr std::array<uint8_t, 6> kElementIdentifier{ 'f', 'l', 'o', 'o', 'r', '1' };

#endif // CONFIG_CHIP

constexpr AccessManager::AccessDocumentRequestParams kAccessDocumentRequestParams{
	.mElementIdentifier = { kElementIdentifier.data(), kElementIdentifier.size() },
	.mIntentToStore = true,
};

constexpr auto kRequiredCapabilities =
	AccessRuleCapabilitiesBits::AccessRuleCapabilitiesBits_Secure_c |
	AccessRuleCapabilitiesBits::AccessRuleCapabilitiesBits_Unsecure_c |
	AccessRuleCapabilitiesBits::AccessRuleCapabilitiesBits_Toggle_Secured_or_Unsecured_c |
	AccessRuleCapabilitiesBits::AccessRuleCapabilitiesBits_Momentary_Unsecure_c |
	AccessRuleCapabilitiesBits::AccessRuleCapabilitiesBits_Extended_Momentary_Unsecure_c |
	AccessRuleCapabilitiesBits::AccessRuleCapabilitiesBits_Payment_Permission_c;

void LogAccessData(const AccessData &accessData)
{
	LOG_DBG("AccessData Version: %d", accessData.AccessData_Version);

	LOG_DBG("AccessData ID present: %d", static_cast<int>(accessData.AccessData_ID_present));
	if (accessData.AccessData_ID_present) {
		const auto &accessDataId = accessData.AccessData_ID.AccessData_ID;
		LOG_DBG("AccessData ID: %.*s", accessDataId.len, accessDataId.value);
	}

	LOG_DBG("AccessData AccessRules present: %d", static_cast<int>(accessData.AccessData_AccessRules_present));
	if (accessData.AccessData_AccessRules_present) {
		const auto &accessDataAccessRules = accessData.AccessData_AccessRules;
		const auto count = accessDataAccessRules.AccessData_AccessRules_AccessRule_m_count;
		for (size_t i = 0; i < count; i++) {
			const auto &accessRule = accessDataAccessRules.AccessData_AccessRules_AccessRule_m[i];
			if (accessRule.AccessRule_Capabilities_present) {
				LOG_DBG("AccessData AccessRule[%d] Capabilities: 0x%02x", i,
					accessRule.AccessRule_Capabilities.AccessRule_Capabilities);
			}
			LOG_DBG("AccessData AccessRule[%d] AllowScheduleIds present: %d, DenyScheduleIds present: %d",
				i, static_cast<int>(accessRule.AccessRule_AllowScheduleIds_present),
				static_cast<int>(accessRule.AccessRule_DenyScheduleIds_present));
		}
	}

	LOG_DBG("AccessData Schedules present: %d", static_cast<int>(accessData.AccessData_Schedules_present));

	LOG_DBG("AccessData ReaderRuleIds present: %d", static_cast<int>(accessData.AccessData_ReaderRuleIds_present));
	if (accessData.AccessData_ReaderRuleIds_present) {
		const auto &readerRuleIds = accessData.AccessData_ReaderRuleIds;
		const auto count = accessData.AccessData_ReaderRuleIds.AccessData_ReaderRuleIds_uint_count;
		for (size_t i = 0; i < count; i++) {
			const auto &readerRuleId = readerRuleIds.AccessData_ReaderRuleIds_uint[i];
			LOG_DBG("AccessData ReaderRuleIds[%d]: %d", i, readerRuleId);
		}
	}

	LOG_DBG("AccessData NonAccessExtensions present: %d",
		static_cast<int>(accessData.AccessData_NonAccessExtensions_present));
	if (accessData.AccessData_NonAccessExtensions_present) {
		const auto &nonAccessExtensions = accessData.AccessData_NonAccessExtensions;
		const auto vendorIdCount = nonAccessExtensions.AccessData_NonAccessExtensions_Vendor_RegisteredID_count;
		for (size_t i = 0; i < vendorIdCount; i++) {
			const auto &vendorRegisteredId =
				nonAccessExtensions.AccessData_NonAccessExtensions_Vendor_RegisteredID[i];
			LOG_DBG("AccessData NonAccessExtensions Vendor Registered ID[%d]: 0x%08x", i,
				vendorRegisteredId.AccessData_NonAccessExtensions_Vendor_RegisteredID_key);

			const auto nonAccessExtensionCount =
				vendorRegisteredId
					.AccessData_NonAccessExtensions_Vendor_RegisteredID_NonAccessExtension_m_count;
			for (size_t j = 0; j < nonAccessExtensionCount; j++) {
				const auto &nonAccessExtension =
					vendorRegisteredId
						.AccessData_NonAccessExtensions_Vendor_RegisteredID_NonAccessExtension_m
							[j];
				LOG_DBG("AccessData NonAccessExtensions[%d] Vendor_ExtensionID: 0x%08x, Version: %d", j,
					nonAccessExtension.NonAccessExtension_Vendor_ExtensionID,
					nonAccessExtension.NonAccessExtension_Version);
				LOG_HEXDUMP_DBG(nonAccessExtension.NonAccessExtension_Data.value,
						nonAccessExtension.NonAccessExtension_Data.len,
						"AccessData NonAccessExtensions Data");
			}
		}
	}

	LOG_DBG("AccessData AccessExtensions present: %d",
		static_cast<int>(accessData.AccessData_AccessExtensions_present));
	if (accessData.AccessData_AccessExtensions_present) {
		const auto &accessExtensions = accessData.AccessData_AccessExtensions;
		const auto vendorIdCount = accessExtensions.AccessData_AccessExtensions_Vendor_RegisteredID_count;
		for (size_t i = 0; i < vendorIdCount; i++) {
			const auto &vendorRegisteredId =
				accessExtensions.AccessData_AccessExtensions_Vendor_RegisteredID[i];
			LOG_DBG("AccessData AccessExtensions Vendor Registered ID[%d]: 0x%08x", i,
				vendorRegisteredId.AccessData_AccessExtensions_Vendor_RegisteredID_key);

			const auto accessExtensionCount =
				vendorRegisteredId
					.AccessData_AccessExtensions_Vendor_RegisteredID_AccessExtension_m_count;
			for (size_t j = 0; j < accessExtensionCount; j++) {
				const auto &accessExtension =
					vendorRegisteredId
						.AccessData_AccessExtensions_Vendor_RegisteredID_AccessExtension_m[j];
				LOG_DBG("AccessData AccessExtensions[%d] Criticality: 0x%02x", j,
					accessExtension.AccessExtension_Criticality);
				LOG_DBG("AccessData AccessExtensions[%d] Vendor_ExtensionID: 0x%08x", j,
					accessExtension.AccessExtension_Vendor_ExtensionID);
				LOG_DBG("AccessData AccessExtensions[%d] Version: %d", j,
					accessExtension.AccessExtension_Version);
				LOG_HEXDUMP_DBG(accessExtension.AccessExtension_Data.value,
						accessExtension.AccessExtension_Data.len,
						"AccessData AccessExtensions Data");
			}
		}
	}
}

bool ValidateAccessData(const ConstData &accessDataBytes)
{
	VerifyOrReturnFalse(accessDataBytes.mData && accessDataBytes.mLength, LOG_ERR("AccessData is empty"));

	LOG_HEXDUMP_DBG(accessDataBytes.mData, accessDataBytes.mLength, "AccessData");

	AccessData accessData{};
	const auto err = cbor_decode_AccessData(accessDataBytes.mData, accessDataBytes.mLength, &accessData, nullptr);
	VerifyOrReturnFalse(err == ZCBOR_SUCCESS, LOG_ERR("Failed to decode AccessData: %d", err));

	VerifyOrReturnFalse(accessData.AccessData_Version == 1,
			    LOG_ERR("Unsupported AccessData Version: %d", accessData.AccessData_Version));

	if (accessData.AccessData_AccessRules_present) {
		bool accessRuleValid{ false };
		const auto count = accessData.AccessData_AccessRules.AccessData_AccessRules_AccessRule_m_count;

		for (size_t i = 0; i < count; i++) {
			const auto &accessRule =
				accessData.AccessData_AccessRules.AccessData_AccessRules_AccessRule_m[i];
			if (accessRule.AccessRule_AllowScheduleIds_present ||
			    accessRule.AccessRule_DenyScheduleIds_present) {
				// Schedules not supported
				continue;
			}

			if (accessRule.AccessRule_Capabilities_present) {
				const auto capabilities = accessRule.AccessRule_Capabilities.AccessRule_Capabilities;
				if ((capabilities & kRequiredCapabilities) == kRequiredCapabilities) {
					accessRuleValid = true;
					// Found a valid access rule
					break;
				}
			}
		}

		VerifyOrReturnFalse(accessRuleValid, LOG_ERR("AccessData AccessRules, no valid rule found"));
	}

	VerifyOrReturnFalse(!accessData.AccessData_Schedules_present, LOG_ERR("AccessData Schedules not supported"));
	VerifyOrReturnFalse(!accessData.AccessData_ReaderRuleIds_present,
			    LOG_ERR("AccessData ReaderRuleIds not supported"));

	if (accessData.AccessData_AccessExtensions_present) {
		const auto &accessExtensions = accessData.AccessData_AccessExtensions;
		const auto vendorIdCount = accessExtensions.AccessData_AccessExtensions_Vendor_RegisteredID_count;
		for (size_t i = 0; i < vendorIdCount; i++) {
			const auto &vendorRegisteredId =
				accessExtensions.AccessData_AccessExtensions_Vendor_RegisteredID[i];
			const auto accessExtensionCount =
				vendorRegisteredId
					.AccessData_AccessExtensions_Vendor_RegisteredID_AccessExtension_m_count;

			for (size_t j = 0; j < accessExtensionCount; j++) {
				const auto &accessExtension =
					vendorRegisteredId
						.AccessData_AccessExtensions_Vendor_RegisteredID_AccessExtension_m[j];

				const auto criticalExtension =
					!IS_BIT_SET(accessExtension.AccessExtension_Criticality,
						    Criticality_Bits::Criticality_Bits_Critical_c);
				VerifyOrReturnFalse(
					!criticalExtension,
					LOG_ERR("AccessData AccessExtensions, critical extensions are not supported"));
			}
		}
	}

	LogAccessData(accessData);

	return true;
}

AliroError GetCurrentValidityIterations(size_t credentialIssuerKeyIndex, ValidityIterations &iterations)
{
	ReturnErrorOnFailure(ReadValidityIterations(credentialIssuerKeyIndex, iterations));

	LOG_DBG("Credential Issuer Key Index: %u, Current Access Iteration: %" PRIu64, credentialIssuerKeyIndex,
		iterations.mAccessIteration);

	return ALIRO_NO_ERROR;
}

bool VerifyValidityIteration(const ValidityIterations &currentIterations, ValidityIteration validityIteration)
{
	LOG_DBG("Access Document Validity Iteration: %" PRIu64, validityIteration);

	if (validityIteration < currentIterations.mAccessIteration) {
		const auto diff = currentIterations.mAccessIteration - validityIteration;
		VerifyOrReturnFalse(diff < kMaxValidityIterationDiff,
				    LOG_WRN("Validity Iteration is too old (diff: %" PRIu64 ")", diff););
	}

	return true;
}

AliroError StoreAccessDocument(size_t keyIndex, size_t credentialIssuerKeyIndex,
			       const AccessDocumentTypes::AccessDocument &accessDocument)
{
	VerifyOrReturnStatus(accessDocument.mDataElement.mLength <= AccessDocument::kAccessDocumentSize,
			     ALIRO_NO_MEMORY, LOG_ERR("Access Document size is too large"));

	AccessDocument ad;
	ad.mVersion = AccessDocument::kVersion;
	ad.mCredentialIssuerKeyIndex = credentialIssuerKeyIndex;
	ad.mSignedTimestamp = accessDocument.mSignedTimestamp;
	ad.mAccessIteration = accessDocument.mValidityIteration.value_or(0);
	ad.mPublicKey = accessDocument.mPublicKey;
	ad.mAccessDocumentSize = accessDocument.mDataElement.mLength;
	std::copy_n(accessDocument.mDataElement.mData, accessDocument.mDataElement.mLength, ad.mAccessDocument.data());
	std::fill(ad.mAccessDocument.begin() + accessDocument.mDataElement.mLength, ad.mAccessDocument.end(), 0);

	LOG_DBG("Storing Access Document at index: %u, Credential Issuer Key Index: %u, Timestamp: %.*s, Validity Iteration: %" PRIu64,
		keyIndex, credentialIssuerKeyIndex, ad.mSignedTimestamp.size(), ad.mSignedTimestamp.data(),
		ad.mAccessIteration);

	ReturnErrorOnFailure(StoreAccessDocument(keyIndex, ad));

	return ALIRO_NO_ERROR;
}

bool IsCurrentAccessDocumentUpToDate(size_t keyIndex, const Timestamp &credentialSignedTimestamp)
{
	AccessDocument ad;
	const auto error = ReadAccessDocument(keyIndex, ad);
	VerifyOrReturnFalse(error == ALIRO_NO_ERROR);

	LOG_DBG("Saved timestamp: %.*s", ad.mSignedTimestamp.size(), ad.mSignedTimestamp.data());
	LOG_DBG("New timestamp  : %.*s", credentialSignedTimestamp.size(), credentialSignedTimestamp.data());

	const auto savedTimestamp = Time::FromTimestamp(ad.mSignedTimestamp);
	VerifyOrReturnFalse(savedTimestamp.has_value(), LOG_ERR("Failed to parse saved timestamp"));

	const auto newTimestamp = Time::FromTimestamp(credentialSignedTimestamp);
	VerifyOrReturnFalse(newTimestamp.has_value(), LOG_ERR("Failed to parse new timestamp"));

	return savedTimestamp.value() >= newTimestamp.value();
}

#endif // CONFIG_DOOR_LOCK_STEP_UP_PHASE

} // namespace

void AccessManagerImpl::_SetApplicationCallbacks(const ApplicationCallbacks &callbacks)
{
	mCallbacks = callbacks;
}

void AccessManagerImpl::_SetStackCallbacks(const StackCallbacks &callbacks)
{
	mStackCallbacks = callbacks;
}

std::optional<AccessManager::AccessDocumentRequestParams> AccessManagerImpl::_ShouldRequestAccessDocument(
	[[maybe_unused]] const CryptoTypes::PublicKey &publicKey,
	[[maybe_unused]] const std::optional<Timestamp> &credentialSignedTimestamp)
{
#ifdef CONFIG_DOOR_LOCK_STEP_UP_PHASE
	MutexGuard lock{ sMutex };
	VerifyOrReturnValue(
		!IsPublicKeyStored(mAcKeys, publicKey), std::nullopt,
		LOG_INF("Provided User Device public key found in Access Manager database, not requesting Access Document"));

	size_t keyIndex{};
	if (IsPublicKeyStored(mAdKeys, publicKey, &keyIndex)) {
		if (credentialSignedTimestamp.has_value()) {
			const auto isUpToDate =
				IsCurrentAccessDocumentUpToDate(keyIndex, credentialSignedTimestamp.value());
			if (!isUpToDate) {
				LOG_INF("User Device has newer Access Document");
				return kAccessDocumentRequestParams;
			} else {
				LOG_INF("Current Access Document is up to date");
				return std::nullopt;
			}
		}

		LOG_INF("Provided User Device public key found in Access Manager database, not requesting Access Document");
		return std::nullopt;
	}

	LOG_INF("Provided User Device public key not found in Access Manager database, requesting Access Document");
	return kAccessDocumentRequestParams;
#endif // CONFIG_DOOR_LOCK_STEP_UP_PHASE

	return std::nullopt;
}

AliroError AccessManagerImpl::_VerifyAccessCredential(
	const CryptoTypes::PublicKey &userPublicKey, bool isNfcSession, SessionContext,
	[[maybe_unused]] const std::optional<AccessDocumentTypes::AccessDocument> &accessDocument)
{
	AliroError status{ ALIRO_NO_ERROR };
	{
		MutexGuard lock{ sMutex };
		status = VerifyPublicKey(userPublicKey) ? ALIRO_NO_ERROR : ALIRO_INVALID_PUBLIC_KEY;
	}

#ifdef CONFIG_DOOR_LOCK_STEP_UP_PHASE
	if (status != ALIRO_NO_ERROR && accessDocument.has_value()) {
		const auto &ad = accessDocument.value();
		status = ProcessAccessDocument(userPublicKey, ad);
	}
#endif // CONFIG_DOOR_LOCK_STEP_UP_PHASE

	HandleAccessGranted(isNfcSession, status == ALIRO_NO_ERROR);
	return status;
}

AliroError AccessManagerImpl::_VerifyKPersistentKey([[maybe_unused]] CryptoTypes::KeyId kpersistentKeyId,
						    [[maybe_unused]] bool isNfcSession, SessionContext)
{
#ifdef CONFIG_DOOR_LOCK_EXPEDITED_FAST_PHASE

	CryptoTypes::PublicKey publicKey{};
	VerifyOrReturnStatus(mKpersistentManager, ALIRO_INVALID_STATE, LOG_ERR("Kpersistent manager not set"));
	AliroError status = mKpersistentManager->GetAccessCredentialPublicKey(kpersistentKeyId, publicKey);
	HandleAccessGranted(isNfcSession, status == ALIRO_NO_ERROR);
	return status;

#else

	return ALIRO_ERROR_NOT_IMPLEMENTED;

#endif // CONFIG_DOOR_LOCK_EXPEDITED_FAST_PHASE
}

#ifdef CONFIG_DOOR_LOCK_BLE_UWB
AliroError AccessManagerImpl::_StartRangingSession(uint32_t rangingSessionId, const CryptoTypes::Ursk &ursk,
						   ProtocolVersion protocolVersion, SessionContext sessionContext)
{
	return AddRangingSession(rangingSessionId, ursk, protocolVersion, sessionContext);
}

#endif // CONFIG_DOOR_LOCK_BLE_UWB

AliroError AccessManagerImpl::_AddPublicKey(const CryptoTypes::PublicKey &publicKey, PublicKeyType publicKeyType,
					    size_t keyIndex)
{
	if (publicKeyType == PublicKeyType::AccessCredential) {
		LOG_DBG("Adding Access Credential public key to storage");
		return AddKeyToContainer(mAcKeys, publicKey, keyIndex);
	}
#if CONFIG_DOOR_LOCK_ACCESS_MANAGER_CREDENTIAL_ISSUER_MAX_STORED_KEYS > 0
	else if (publicKeyType == PublicKeyType::CredentialIssuer) {
		LOG_DBG("Adding Credential Issuer public key to storage");
		return AddKeyToContainer(mCiKeys, publicKey, keyIndex);
	}
#endif // CONFIG_DOOR_LOCK_ACCESS_MANAGER_CREDENTIAL_ISSUER_MAX_STORED_KEYS > 0
#if CONFIG_DOOR_LOCK_STORAGE_MAX_STORED_ACCESS_DOCUMENTS > 0
	else if (publicKeyType == PublicKeyType::AccessDocument) {
		LOG_DBG("Adding Access Document public key to storage");
		return AddKeyToContainer(mAdKeys, publicKey, keyIndex);
	}
#endif // CONFIG_DOOR_LOCK_STORAGE_MAX_STORED_ACCESS_DOCUMENTS > 0

	LOG_WRN("Invalid public key type");
	return ALIRO_INVALID_ARGUMENT;
}

AliroError AccessManagerImpl::_RemovePublicKey(PublicKeyType publicKeyType, size_t keyIndex)
{
	if (publicKeyType == PublicKeyType::AccessCredential) {
		LOG_DBG("Removing Access Credential public key from storage");
		ReturnErrorOnFailure(RemoveKeyFromContainer(mAcKeys, keyIndex));

#ifdef CONFIG_DOOR_LOCK_EXPEDITED_FAST_PHASE

		if (mKpersistentManager) {
			mKpersistentManager->RemoveKpersistent(keyIndex);
		}

#endif // CONFIG_DOOR_LOCK_EXPEDITED_FAST_PHASE

		return ALIRO_NO_ERROR;

	}
#if CONFIG_DOOR_LOCK_ACCESS_MANAGER_CREDENTIAL_ISSUER_MAX_STORED_KEYS > 0
	else if (publicKeyType == PublicKeyType::CredentialIssuer) {
		LOG_DBG("Removing Credential Issuer public key from storage");
		ReturnErrorOnFailure(RemoveKeyFromContainer(mCiKeys, keyIndex));
		ReturnErrorOnFailure(RemoveAccessCredentials(keyIndex));
		return ALIRO_NO_ERROR;
	}
#endif // CONFIG_DOOR_LOCK_ACCESS_MANAGER_CREDENTIAL_ISSUER_MAX_STORED_KEYS > 0
#if CONFIG_DOOR_LOCK_STORAGE_MAX_STORED_ACCESS_DOCUMENTS > 0
	else if (publicKeyType == PublicKeyType::AccessDocument) {
		LOG_DBG("Removing Access Document public key from storage");
		ReturnErrorOnFailure(RemoveKeyFromContainer(mAdKeys, keyIndex));
		ReturnErrorOnFailure(ClearAccessDocument(keyIndex));

#ifdef CONFIG_DOOR_LOCK_EXPEDITED_FAST_PHASE
		if (mKpersistentManager) {
			const auto index = keyIndex + CONFIG_DOOR_LOCK_ACCESS_MANAGER_ACCESS_CREDENTIAL_MAX_STORED_KEYS;
			mKpersistentManager->RemoveKpersistent(index);
		}
#endif // CONFIG_DOOR_LOCK_EXPEDITED_FAST_PHASE

		return ALIRO_NO_ERROR;
	}
#endif // CONFIG_DOOR_LOCK_STORAGE_MAX_STORED_ACCESS_DOCUMENTS > 0

	LOG_WRN("Invalid public key type");
	return ALIRO_INVALID_ARGUMENT;
}

void AccessManagerImpl::_ClearStoredKeys()
{
	MutexGuard lock{ sMutex };

#ifdef CONFIG_DOOR_LOCK_EXPEDITED_FAST_PHASE

	if (mKpersistentManager) {
		mKpersistentManager->RemoveAllKpersistent();
	}

#endif // CONFIG_DOOR_LOCK_EXPEDITED_FAST_PHASE

	for (auto &key : mAcKeys.mKeys) {
		key.reset();
	}
	mAcKeys.mCount = 0;

	for (auto &key : mCiKeys.mKeys) {
		key.reset();
	}
	mCiKeys.mCount = 0;

	for (auto &key : mAdKeys.mKeys) {
		key.reset();
	}
	mAdKeys.mCount = 0;

	LOG_INF("Cleared all stored public keys");
}

void AccessManagerImpl::_SetMaxAllowedDistance(uint32_t maxDistance)
{
#ifdef CONFIG_DOOR_LOCK_BLE_UWB
	MutexGuard lock{ sMutex };

	mMaxAllowedDistance = maxDistance;
	LOG_INF("Set maximum allowed distance to %u cm", maxDistance);
#else // CONFIG_DOOR_LOCK_BLE_UWB

	ARG_UNUSED(maxDistance);
#endif // CONFIG_DOOR_LOCK_BLE_UWB
}

uint32_t AccessManagerImpl::_GetMaxAllowedDistance()
{
#ifdef CONFIG_DOOR_LOCK_BLE_UWB
	MutexGuard lock{ sMutex };

	return mMaxAllowedDistance;
#else
	return 0;
#endif // CONFIG_DOOR_LOCK_BLE_UWB
}

void AccessManagerImpl::_HandleRangingSessionStateChanged(SessionContext sessionContext, RangingSessionState state)
{
#ifdef CONFIG_DOOR_LOCK_BLE_UWB
	switch (state) {
	case RangingSessionState::Ranging:
		LOG_INF("Ranging state changed to Ranging (session: %p)", sessionContext);
		break;
	case RangingSessionState::RangingSuspended:
		LOG_INF("Ranging state changed to Ranging Suspended (session: %p)", sessionContext);

		// To prevent rapid toggling after Suspend event, only update ReaderState if no other sessions are in range
		SetInRangeState(sessionContext, false, !IsUserDeviceInRange());
		break;
	case RangingSessionState::RangingResumed:
		LOG_INF("Ranging state changed to Ranging Resumed (session: %p)", sessionContext);
		break;
	case RangingSessionState::Destroyed:
		LOG_INF("Ranging state changed to Destroyed (session: %p)", sessionContext);
		// Only update ReaderState if no other sessions are in range
		SetInRangeState(sessionContext, false);
		break;
	default:
		break;
	}

#else // CONFIG_DOOR_LOCK_BLE_UWB
	ARG_UNUSED(sessionContext);
	ARG_UNUSED(state);
#endif // CONFIG_DOOR_LOCK_BLE_UWB
}

void AccessManagerImpl::_HandleRangingSessionData(SessionContext sessionContext, const UwbRangingData &uwbData)
{
	LOG_DBG("Handling ranging session data - length: %u for session: %p", uwbData.mLength, sessionContext);

#ifdef CONFIG_DOOR_LOCK_BLE_UWB
	const auto currentSessionInRange = AnalyzeUwbRangingData(uwbData, sessionContext);
	SetInRangeState(sessionContext, currentSessionInRange);
#endif // CONFIG_DOOR_LOCK_BLE_UWB
}

void AccessManagerImpl::_HandleSessionTermination(SessionContext sessionContext, [[maybe_unused]] bool isNfcSession)
{
	VerifyOrReturn(sessionContext, LOG_ERR("Session context is null"));

	LOG_INF("Handling session termination");

#ifdef CONFIG_DOOR_LOCK_BLE_UWB
	if (isNfcSession) {
		return;
	}

	SetInRangeState(sessionContext, false);
	RemoveRangingSession(sessionContext);

#endif // CONFIG_DOOR_LOCK_BLE_UWB
}

bool AccessManagerImpl::VerifyPublicKey(const CryptoTypes::PublicKey &userPublicKey) const
{
	LOG_DBG("Verifying public key against %u stored keys", mAcKeys.mCount);

	// Check if the user's public key matches any stored key
	VerifyOrReturnFalse(IsPublicKeyStored(mAcKeys, userPublicKey),
			    LOG_INF("Provided User Device public key not found in Access Manager database"));

	return true;
}

bool AccessManagerImpl::_IsPublicKeyStored(const CryptoTypes::PublicKey &userPublicKey, size_t *keyIndex)
{
	MutexGuard lock{ sMutex };

	auto hasKey = IsPublicKeyStored(mAcKeys, userPublicKey, keyIndex);
	if (hasKey) {
		return true;
	}

	hasKey = IsPublicKeyStored(mAdKeys, userPublicKey, keyIndex);
	if (!hasKey) {
		return false;
	}

	if (keyIndex) {
		// offset the key index by the number of AC keys
		*keyIndex += mAcKeys.mKeys.size();
	}

	return true;
}

template <size_t T>
bool AccessManagerImpl::IsPublicKeyStored(const StoredKeys<T> &container, const CryptoTypes::PublicKey &userPublicKey,
					  size_t *keyIndex) const
{
	for (size_t id = 0; id < container.mKeys.size(); id++) {
		if (container.mKeys[id] == userPublicKey) {
			if (keyIndex) {
				*keyIndex = id;
			}
			return true;
		}
	}

	return false;
}

template <size_t T> AliroError AccessManagerImpl::GetFirstFreeIndex(StoredKeys<T> &container, size_t &keyIndex) const
{
	for (size_t id = 0; id < container.mKeys.size(); id++) {
		if (!container.mKeys[id].has_value()) {
			keyIndex = id;
			return ALIRO_NO_ERROR;
		}
	}

	return ALIRO_NO_MEMORY;
}

AliroError AccessManagerImpl::_GetPublicKey(size_t keyIndex, CryptoTypes::PublicKey &publicKey)
{
	MutexGuard lock{ sMutex };
	VerifyOrReturnStatus(keyIndex < mAcKeys.mKeys.size() + mAdKeys.mKeys.size(), ALIRO_INVALID_ARGUMENT,
			     LOG_WRN("Key index out of range"));

	if (keyIndex < mAcKeys.mKeys.size()) {
		VerifyOrReturnStatus(mAcKeys.mKeys[keyIndex].has_value(), ALIRO_INVALID_ARGUMENT,
				     LOG_WRN("Key not found in storage"));
		publicKey = mAcKeys.mKeys[keyIndex].value();
	} else {
		const auto index = keyIndex - mAcKeys.mKeys.size();
		VerifyOrReturnStatus(mAdKeys.mKeys[index].has_value(), ALIRO_INVALID_ARGUMENT,
				     LOG_WRN("Key not found in storage"));
		publicKey = mAdKeys.mKeys[index].value();
	}

	return ALIRO_NO_ERROR;
}

void AccessManagerImpl::UnlockAction() const
{
	VerifyAndCall(mCallbacks.mUnlockIndicatorClb);
}

void AccessManagerImpl::LockAction() const
{
	VerifyAndCall(mCallbacks.mLockIndicatorClb);
}

void AccessManagerImpl::AccessGrantedAction(bool isNfcSession) const
{
	LOG_INF("ACCESS GRANTED");
	VerifyAndCall(mCallbacks.mAccessIndicatorClb, true, isNfcSession);
}

void AccessManagerImpl::AccessDeniedAction(bool isNfcSession) const
{
	LOG_INF("ACCESS DENIED");
	VerifyAndCall(mCallbacks.mAccessIndicatorClb, false, isNfcSession);
}

void AccessManagerImpl::HandleAccessGranted(bool isNfcSession, bool granted)
{
	if (granted) {
		AccessGrantedAction(isNfcSession);
		if (ShouldUnlockImmediately(isNfcSession)) {
			UnlockAction();
		}
	} else {
		AccessDeniedAction(isNfcSession);
	}
}

bool AccessManagerImpl::ShouldUnlockImmediately(bool isNfcSession) const
{
	VerifyOrReturnFalse(isNfcSession);

#ifdef CONFIG_DOOR_LOCK_BLE_UWB

	// Skip Reader Status Changed Message for NFC sessions when Reader is opened via BLE+UWB
	if (!IsUserDeviceInRange()) {
		Aliro::AliroStack::Instance().SendReaderStatusChangedMessage(OperationSource::ThisUserDeviceInNfc,
									     ReaderStateByte::Unsecured);
	}

	// For NFC sessions with UWB enabled, only unlock if not already in range
	// This prevents double-unlocking when user is already detected via UWB
	return !IsUserDeviceInRange();

#else // CONFIG_DOOR_LOCK_BLE_UWB

	// For NFC sessions without UWB, always unlock immediately
	return true;

#endif // CONFIG_DOOR_LOCK_BLE_UWB
}

#ifdef CONFIG_DOOR_LOCK_BLE_UWB
bool AccessManagerImpl::AnalyzeUwbRangingData(const UwbRangingData &uwbData, SessionContext sessionContext)
{
	LOG_DBG("Analyzing UWB ranging data - length: %u", uwbData.mLength);

	// Check if UWB data is valid
	VerifyOrReturnFalse(uwbData.mData && uwbData.mLength, LOG_WRN("Invalid UWB ranging data"));

	// Extract distance from UWB data
	auto distance = ExtractDistanceFromUwbData(uwbData);

	VerifyOrReturnFalse(distance, LOG_WRN("Failed to extract distance from UWB data"));

	MutexGuard lock{ sMutex };

	// Find the session
	auto *sessionCtx = FindRangingSession(sessionContext);
	VerifyOrReturnFalse(sessionCtx, LOG_ERR("Session context not found for handle: %p", sessionContext));

	// Apply exit margin logic based on session's previous state:
	// - Unlock (enter range): distance <= mMaxAllowedDistance (when session was out of range)
	// - Lock (exit range): distance > mMaxAllowedDistance + mMaxAllowedDistanceExitMargin (when session was in range)
	// Use session's previous state (sessionCtx->mInRange) to determine threshold.
	// This ensures that each session independently tracks its state transitions and prevents
	// issues when sessions are created/destroyed (e.g., BLE disconnect/reconnect).
	// The exit margin is only applied when the door is unlocked to prevent rapid toggling.
	const bool sessionWasInRange = sessionCtx->mInRange;
	const uint32_t threshold = sessionWasInRange ? (mMaxAllowedDistance + mMaxAllowedDistanceExitMargin) : mMaxAllowedDistance;

	LOG_DBG("Extracted distance: %u cm, threshold: %u cm (max: %u cm, exit margin: %u cm, sessionWasInRange: %d)",
		distance.value(), threshold, mMaxAllowedDistance, mMaxAllowedDistanceExitMargin, static_cast<int>(sessionWasInRange));

	// Check if distance is within acceptable range
	VerifyOrReturnFalse(distance.value() <= threshold,
			    LOG_DBG("Distance check failed - User Device is too far"));

	LOG_DBG("Distance check passed, User Device is within range");
	return true;
}

std::optional<uint16_t> AccessManagerImpl::ExtractDistanceFromUwbData(const UwbRangingData &uwbData) const
{
	// TODO: Implement the actual UWB ranging data format
	// For now, we'll assume the distance is in the first 2 bytes of the UWB data
	VerifyOrReturnValue(uwbData.mLength == sizeof(uint16_t), std::nullopt, LOG_ERR("Invalid UWB data length"));
	return std::make_optional<uint16_t>(sys_get_be16(uwbData.mData));
}

AliroError AccessManagerImpl::AddRangingSession(uint32_t rangingSessionId, const CryptoTypes::Ursk &ursk,
						ProtocolVersion protocolVersion, SessionContext sessionCtx)
{
	auto *newCtx = Aliro::new_nothrow<RangingSessionContext>(
#ifdef CONFIG_DOOR_LOCK_ACCESS_MANAGER_TERMINATE_SESSION_ON_TIMEOUT
		CONFIG_DOOR_LOCK_ACCESS_MANAGER_SESSION_TIMEOUT_MS,
		[](Timer::Context ctx) { AccessManagerImpl::Instance().TerminateAliroSession(ctx); },
		const_cast<Timer::Context>(sessionCtx)
#endif // CONFIG_DOOR_LOCK_ACCESS_MANAGER_TERMINATE_SESSION_ON_TIMEOUT
	);
	VerifyOrReturnStatus(newCtx, ALIRO_NO_MEMORY, LOG_ERR("Cannot allocate context for UWB session."));

	newCtx->mInRange = false;
	newCtx->mSessionContext = sessionCtx;

	MutexGuard lock{ sMutex };

	AliroError status = Uwb::UltraWideBandImpl::Instance().ConfigureRangingSession(rangingSessionId, ursk,
										       protocolVersion, sessionCtx);
	VerifyOrExit(status == ALIRO_NO_ERROR, LOG_ERR("Failed to configure ranging session: %d", status.ToInt()));

	sys_slist_append(&mActiveSessions, &newCtx->mNode);

	return ALIRO_NO_ERROR;

exit:
#ifdef CONFIG_DOOR_LOCK_ACCESS_MANAGER_TERMINATE_SESSION_ON_TIMEOUT
	newCtx->mRangingSessionTimer.Stop();
#endif // CONFIG_DOOR_LOCK_ACCESS_MANAGER_TERMINATE_SESSION_ON_TIMEOUT
	delete newCtx;

	return status;
}

void AccessManagerImpl::RemoveRangingSession(SessionContext sessionCtx)
{
	MutexGuard lock{ sMutex };

	sys_snode_t *node{ nullptr };
	sys_snode_t *nodeSafe{ nullptr };
	sys_snode_t *prevNode{ nullptr };

	AliroError status = Uwb::UltraWideBandImpl::Instance().TerminateRangingSession(sessionCtx);
	VerifyOrReturn(status == ALIRO_NO_ERROR || status == ALIRO_ERROR_NOT_IMPLEMENTED,
		       LOG_ERR("Cannot terminate UWB ranging session: %d", status.ToInt()));

	SYS_SLIST_FOR_EACH_NODE_SAFE (&mActiveSessions, node, nodeSafe) {
		auto *ctx = CONTAINER_OF(node, RangingSessionContext, mNode);
		if (ctx->mSessionContext == sessionCtx) {
			sys_slist_remove(&mActiveSessions, prevNode, &ctx->mNode);
#ifdef CONFIG_DOOR_LOCK_ACCESS_MANAGER_TERMINATE_SESSION_ON_TIMEOUT
			ctx->mRangingSessionTimer.Stop();
#endif // CONFIG_DOOR_LOCK_ACCESS_MANAGER_TERMINATE_SESSION_ON_TIMEOUT
			delete ctx;
			break;
		}
		prevNode = node;
	}
}

AccessManagerImpl::RangingSessionContext *AccessManagerImpl::FindRangingSession(const SessionContext sessionCtx)
{
	RangingSessionContext *rangingSessionCtx{};

	MutexGuard lock{ sMutex };
	SYS_SLIST_FOR_EACH_CONTAINER (&mActiveSessions, rangingSessionCtx, mNode) {
		if (rangingSessionCtx->mSessionContext == sessionCtx) {
			return rangingSessionCtx;
		}
	}

	return nullptr;
}

bool AccessManagerImpl::IsUserDeviceInRange() const
{
	RangingSessionContext *rangingSessionCtx{};

	MutexGuard lock{ sMutex };
	SYS_SLIST_FOR_EACH_CONTAINER (&mActiveSessions, rangingSessionCtx, mNode) {
		if (rangingSessionCtx->mInRange) {
			return true;
		}
	}

	return false;
}

void AccessManagerImpl::SetInRangeState(SessionContext sessionContext, bool sessionInRange, bool updateReaderState)
{
#ifdef CONFIG_DOOR_LOCK_BLE_UWB
	RangingSessionContext *sessionCtx{ nullptr };
	bool wasAnySessionInRange{ false };
	{
		MutexGuard lock{ sMutex };
		// Find the session
		sessionCtx = FindRangingSession(sessionContext);
		VerifyOrReturn(sessionCtx, LOG_ERR("Session context not found for handle: %p", sessionContext));

		// Early return if has not changed
		if (sessionCtx->mInRange == sessionInRange) {
			return;
		}

		// Read wasAnySessionInRange before updating session state to get accurate previous state
		wasAnySessionInRange = IsUserDeviceInRange();

		// Update the session state
		sessionCtx->mInRange = sessionInRange;

#ifdef CONFIG_DOOR_LOCK_ACCESS_MANAGER_TERMINATE_SESSION_ON_TIMEOUT
		// Start timer if session is in range and timer is not running
		if (sessionInRange && !sessionCtx->mRangingSessionTimer.IsRunning()) {
			sessionCtx->mRangingSessionTimer.Start();
		}
#endif // CONFIG_DOOR_LOCK_ACCESS_MANAGER_TERMINATE_SESSION_ON_TIMEOUT
	}

	bool nowAnySessionInRange{ false };
	nowAnySessionInRange = sessionInRange || (wasAnySessionInRange && IsUserDeviceInRange());

	// Handle state change and trigger appropriate actions if state changed
	if (updateReaderState && (nowAnySessionInRange != wasAnySessionInRange)) {
#ifdef CONFIG_DOOR_LOCK_ACCESS_MANAGER_TERMINATE_SESSION_ON_ACCESS_GRANTED
		if (wasAnySessionInRange && nowAnySessionInRange) {
			TerminateAliroSession(sessionContext);
		}
#endif // CONFIG_DOOR_LOCK_ACCESS_MANAGER_TERMINATE_SESSION_ON_ACCESS_GRANTED

		auto readerState = nowAnySessionInRange ? ReaderStateByte::Unsecured : ReaderStateByte::Secured;

		Aliro::AliroStack::Instance().SendReaderStatusChangedMessage(
			OperationSource::ThisUserDeviceInBluetoothLeUwbAliroFlow, readerState);

		if (nowAnySessionInRange) {
			UnlockAction();
#ifdef CONFIG_DOOR_LOCK_ACCESS_MANAGER_TERMINATE_SESSION_ON_ACCESS_GRANTED
			TerminateAliroSession(sessionContext);
#endif // CONFIG_DOOR_LOCK_ACCESS_MANAGER_TERMINATE_SESSION_ON_ACCESS_GRANTED
		} else {
			LockAction();
		}
	}
#endif // CONFIG_DOOR_LOCK_BLE_UWB
}

void AccessManagerImpl::TerminateAliroSession(SessionContext sessionContext)
{
	LOG_DBG("Terminating Aliro session for context: %p", sessionContext);
	VerifyAndCall(mStackCallbacks.mTerminateSessionClb, sessionContext);
}
#endif // CONFIG_DOOR_LOCK_BLE_UWB

template <size_t T>
AliroError AccessManagerImpl::RemoveKeyFromContainer(StoredKeys<T> &container, size_t keyIndex) const
{
	MutexGuard lock{ sMutex };

	VerifyOrReturnStatus(keyIndex < container.mKeys.size(), ALIRO_INVALID_ARGUMENT,
			     LOG_WRN("Key index out of range"));
	VerifyOrReturnStatus(container.mKeys[keyIndex].has_value(), ALIRO_PUBLIC_KEY_NOT_FOUND,
			     LOG_WRN("Public key not found in storage"));

	container.mKeys[keyIndex].reset();
	container.mCount--;

	LOG_INF("Removed public key from container. Total keys: %u", container.mCount);

	return ALIRO_NO_ERROR;
}

template <size_t T>
AliroError AccessManagerImpl::AddKeyToContainer(StoredKeys<T> &container, const CryptoTypes::PublicKey &publicKey,
						size_t keyIndex) const
{
	MutexGuard lock{ sMutex };

	VerifyOrReturnStatus(keyIndex < container.mKeys.size(), ALIRO_INVALID_ARGUMENT,
			     LOG_WRN("Key index out of range"));

	// Check if key already exists, if so, just do nothing
	VerifyOrReturnStatus(!container.mKeys[keyIndex].has_value(), ALIRO_NO_ERROR,
			     LOG_WRN("Public key already exists in storage"));

	// Add the key to storage
	container.mKeys[keyIndex] = publicKey;
	container.mCount++;

	LOG_INF("Added public key to slot %u. Total keys: %u", keyIndex, container.mCount);

	return ALIRO_NO_ERROR;
}

AliroError AccessManagerImpl::_GetCredentialIssuerPublicKey(const CryptoTypes::KeyIdentifier &keyIdentifier,
							    CryptoTypes::PublicKey &publicKey) const
{
	constexpr size_t kKeyIdentifierStringLength{ sizeof("key-identifier") - 1 };
	using ShaInputArray = std::array<uint8_t, kKeyIdentifierStringLength + CryptoTypes::kEccP256PublicKeyLength>;

	MutexGuard lock{ sMutex };
	VerifyOrReturnStatus(mCiKeys.mCount > 0, ALIRO_PUBLIC_KEY_NOT_FOUND, LOG_DBG("CI storage is empty"));
	// The "key-identifier" string is fixed, set it once for the SHA256 hash computation
	static ShaInputArray input{ 'k', 'e', 'y', '-', 'i', 'd', 'e', 'n', 't', 'i', 'f', 'i', 'e', 'r' };
	CryptoTypes::Sha256Hash sha256Output;

	for (auto &key : mCiKeys.mKeys) {
		if (!key.has_value()) {
			continue;
		}

		std::copy(key.value().begin(), key.value().end(), input.begin() + kKeyIdentifierStringLength);

		AliroError error = CryptoInstance().Sha256(input.data(), input.size(), sha256Output);
		VerifyOrReturnStatus(error == ALIRO_NO_ERROR, ALIRO_ERROR_INTERNAL,
				     LOG_ERR("SHA256 hash computation failed"));

		// Aliro Specification 7.2.1 Cryptographic requirements: compare the key identifier with the
		// first 8 bytes of the SHA256 hash output
		if (std::equal(keyIdentifier.begin(), keyIdentifier.end(), sha256Output.begin())) {
			publicKey = key.value();
			return ALIRO_NO_ERROR;
		}
	}

	return ALIRO_PUBLIC_KEY_NOT_FOUND;
}

#ifdef CONFIG_DOOR_LOCK_STEP_UP_PHASE

AliroError AccessManagerImpl::ProcessAccessDocument(const CryptoTypes::PublicKey &userPublicKey,
						    const AccessDocumentTypes::AccessDocument &ad)
{
	ReturnErrorOnFailure(ProcessValidityIteration(ad.mCredentialIssuerPublicKey, ad.mValidityIteration));

	VerifyOrReturnStatus(ValidateAccessData(ad.mDataElement), ALIRO_INVALID_ACCESS_DOCUMENT,
			     LOG_WRN("Access Document is not valid, ignoring it for access decision"));

	LOG_DBG("Verify Access Credential based on Access Document");
	VerifyOrReturnStatus(ad.mPublicKey == userPublicKey, ALIRO_INVALID_PUBLIC_KEY, LOG_WRN("Public key mismatch"));

	MutexGuard lock{ sMutex };

	// Check if there is a free index in the Access Document container to store the Access Document.
	size_t keyIndex = 0;
	auto error = GetFirstFreeIndex(mAdKeys, keyIndex);

	if (error == ALIRO_NO_ERROR) {
		size_t credentialIssuerKeyIndex{};
		const bool credentialIssuerKeyStored =
			IsPublicKeyStored(mCiKeys, ad.mCredentialIssuerPublicKey, &credentialIssuerKeyIndex);

		if (credentialIssuerKeyStored) {
			error = StoreAccessDocument(keyIndex, credentialIssuerKeyIndex, ad);
			if (error == ALIRO_NO_ERROR) {
				AddKeyToContainer(mAdKeys, userPublicKey, keyIndex);
			}
		}
	}

	return ALIRO_NO_ERROR;
}

AliroError AccessManagerImpl::ProcessValidityIteration(const CryptoTypes::PublicKey &credentialIssuerPublicKey,
						       const std::optional<ValidityIteration> &validityIteration)
{
	VerifyOrReturnStatus(validityIteration.has_value(), ALIRO_NO_ERROR,
			     LOG_DBG("Validity iteration is not present"));

	// Check if the Credential Issuer public key is stored in the storage.
	// If not, we can ignore the validity iteration, until storing the Credential Issuer public keys
	// from certificate is supported.
	size_t keyIndex{};
	VerifyOrReturnStatus(IsPublicKeyStored(mCiKeys, credentialIssuerPublicKey, &keyIndex), ALIRO_NO_ERROR,
			     LOG_INF("Credential Issuer public key not found in storage"));

	ValidityIterations iterations{};
	ReturnErrorOnFailure(GetCurrentValidityIterations(keyIndex, iterations));

	VerifyOrReturnStatus(VerifyValidityIteration(iterations, validityIteration.value()),
			     ALIRO_INVALID_ACCESS_DOCUMENT,
			     LOG_WRN("Validity Iteration is not valid, ignoring Access Document for access decision"));

	ReturnErrorOnFailure(UpdateValidityIteration(keyIndex, iterations, validityIteration.value()));

	return ALIRO_NO_ERROR;
}

AliroError AccessManagerImpl::UpdateValidityIteration(size_t credentialIssuerKeyIndex,
						      const ValidityIterations &currentIterations,
						      ValidityIteration validityIteration)
{
	ValidityIterations iterations = currentIterations;

	if (validityIteration > iterations.mAccessIteration) {
		iterations.mAccessIteration = validityIteration;
	}

	if (iterations != currentIterations) {
		LOG_DBG("Updating Validity Iterations for Credential Issuer Key Index: %u, New Access Iteration: %" PRIu64,
			credentialIssuerKeyIndex, iterations.mAccessIteration);

		ReturnErrorOnFailure(StoreValidityIterations(credentialIssuerKeyIndex, iterations));
	}

	ReturnErrorOnFailure(RemoveOldCredentials(credentialIssuerKeyIndex, iterations.mAccessIteration));

	return ALIRO_NO_ERROR;
}

AliroError AccessManagerImpl::RemoveOldCredentials(size_t credentialIssuerKeyIndex, ValidityIteration validityIteration)
{
	for (size_t i = 0; i < mAdKeys.mKeys.size(); i++) {
		if (mAdKeys.mKeys[i].has_value()) {
			AccessDocument ad;
			auto error = ReadAccessDocument(i, ad);
			if (error != ALIRO_NO_ERROR) {
				continue;
			}

			if (ad.mCredentialIssuerKeyIndex != credentialIssuerKeyIndex) {
				continue;
			}

			if (ad.mAccessIteration >= validityIteration) {
				continue;
			}

			const auto diff = validityIteration - ad.mAccessIteration;
			if (diff > kMaxValidityIterationDiff) {
				LOG_DBG("Removing old credentials with index %u due to Validity Iteration difference: %" PRIu64,
					i, diff);
				ReturnErrorOnFailure(_RemovePublicKey(PublicKeyType::AccessDocument, i));
			}
		}
	}

	return ALIRO_NO_ERROR;
}

AliroError AccessManagerImpl::RemoveAccessCredentials(size_t credentialIssuerKeyIndex)
{
	LOG_DBG("Removing Access Credentials associated with Credential Issuer Key Index: %u",
		credentialIssuerKeyIndex);

	for (size_t i = 0; i < mAdKeys.mKeys.size(); i++) {
		if (mAdKeys.mKeys[i].has_value()) {
			AccessDocument ad;
			auto error = ReadAccessDocument(i, ad);
			if (error != ALIRO_NO_ERROR) {
				continue;
			}

			if (ad.mCredentialIssuerKeyIndex != credentialIssuerKeyIndex) {
				continue;
			}

			ReturnErrorOnFailure(_RemovePublicKey(PublicKeyType::AccessDocument, i));
		}
	}

	return ALIRO_NO_ERROR;
}

#endif // CONFIG_DOOR_LOCK_STEP_UP_PHASE

} // namespace Aliro
