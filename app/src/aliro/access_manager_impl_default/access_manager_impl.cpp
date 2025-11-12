/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "access_manager_impl.h"
#include "aliro/aliro.h"
#include "aliro/mutex_guard.h"
#include "aliro/utils.h"
#include "crypto/crypto.h"

#ifdef CONFIG_DOOR_LOCK_STEP_UP_PHASE
#include "cbor/access_document_decode.h"
#endif // CONFIG_DOOR_LOCK_STEP_UP_PHASE

#ifdef CONFIG_ALIRO_BLE_UWB
#include "aliro/memory.h"
#include "uwb_impl.h"
#endif // CONFIG_ALIRO_BLE_UWB

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/byteorder.h>

#include <cstddef>
#include <cstdint>

LOG_MODULE_REGISTER(access_manager, CONFIG_NCS_DOOR_LOCK_APP_LOG_LEVEL);

namespace Aliro {

namespace {

K_MUTEX_DEFINE(sMutex);

#ifdef CONFIG_DOOR_LOCK_STEP_UP_PHASE

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

bool ValidateAccessData(const Data &accessDataBytes)
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

bool AccessManagerImpl::_ShouldRequestAccessDocument([[maybe_unused]] const CryptoTypes::PublicKey &userPublicKey)
{
#ifdef CONFIG_DOOR_LOCK_STEP_UP_PHASE
	MutexGuard lock{ sMutex };
	VerifyOrReturnTrue(
		IsPublicKeyStored(mAcKeys, userPublicKey),
		LOG_INF("Provided User Device public key not found in Access Manager database, requesting access document"));

	LOG_INF("Provided User Device public key found in Access Manager database, not requesting access document");
#endif // CONFIG_DOOR_LOCK_STEP_UP_PHASE

	return false;
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
		const auto accessDocumentValid = ValidateAccessData(accessDocument.value().mDataElement);

		if (accessDocumentValid) {
			LOG_DBG("Verify AC based on Access Document");
			status = accessDocument.value().mPublicKey == userPublicKey ? ALIRO_NO_ERROR :
										      ALIRO_INVALID_PUBLIC_KEY;
		} else {
			LOG_WRN("Access Document is not valid, ignoring it for access decision");
			status = ALIRO_INVALID_ACCESS_DOCUMENT;
		}
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
	if (status == ALIRO_NO_ERROR) {
		HandleAccessGranted(isNfcSession, true);
		return ALIRO_NO_ERROR;
	} else {
		HandleAccessGranted(isNfcSession, false);
		return status;
	}

#else

	return ALIRO_ERROR_NOT_IMPLEMENTED;

#endif // CONFIG_DOOR_LOCK_EXPEDITED_FAST_PHASE
}

#ifdef CONFIG_ALIRO_BLE_UWB
AliroError AccessManagerImpl::_StartRangingSession(uint32_t rangingSessionId, const CryptoTypes::Ursk &ursk,
						   SessionContext sessionContext)
{
	return AddRangingSession(rangingSessionId, ursk, sessionContext);
}

#endif // CONFIG_ALIRO_BLE_UWB

AliroError AccessManagerImpl::_AddPublicKey(const CryptoTypes::PublicKey &publicKey, PublicKeyType publicKeyType,
					    std::optional<size_t> keyIndex)
{
	if (publicKeyType == PublicKeyType::AccessCredential) {
		LOG_DBG("Adding Access Credential public key to storage");
		return AddKeyToContainer(mAcKeys, publicKey, keyIndex);
	}
#if CONFIG_ALIRO_CREDENTIAL_ISSUER_MAX_STORED_KEYS > 0

	else if (publicKeyType == PublicKeyType::CredentialIssuer) {
		LOG_DBG("Adding Credential Issuer public key to storage");
		return AddKeyToContainer(mCiKeys, publicKey, keyIndex);
	}

#endif // CONFIG_ALIRO_CREDENTIAL_ISSUER_MAX_STORED_KEYS > 0

	LOG_WRN("Invalid public key type");
	return ALIRO_INVALID_ARGUMENT;
}

AliroError AccessManagerImpl::_RemovePublicKey(const CryptoTypes::PublicKey &publicKey, PublicKeyType publicKeyType)
{
	size_t keyIndex{};
	if (publicKeyType == PublicKeyType::AccessCredential) {
		LOG_DBG("Removing Access Credential public key from storage");
		ReturnErrorOnFailure(RemoveKeyFromContainer(mAcKeys, publicKey, keyIndex));

#ifdef CONFIG_DOOR_LOCK_EXPEDITED_FAST_PHASE

		if (mKpersistentManager) {
			mKpersistentManager->RemoveKpersistent(keyIndex);
		}

#endif // CONFIG_DOOR_LOCK_EXPEDITED_FAST_PHASE

		return ALIRO_NO_ERROR;

	}
#if CONFIG_ALIRO_CREDENTIAL_ISSUER_MAX_STORED_KEYS > 0
	else if (publicKeyType == PublicKeyType::CredentialIssuer) {
		LOG_DBG("Removing Credential Issuer public key from storage");
		return RemoveKeyFromContainer(mCiKeys, publicKey, keyIndex);
	}

#endif // CONFIG_ALIRO_CREDENTIAL_ISSUER_MAX_STORED_KEYS > 0

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

	LOG_INF("Cleared all stored public keys");
}

void AccessManagerImpl::_SetMaxAllowedDistance(uint32_t maxDistance)
{
#ifdef CONFIG_ALIRO_BLE_UWB
	MutexGuard lock{ sMutex };

	mMaxAllowedDistance = maxDistance;
	LOG_INF("Set maximum allowed distance to %u cm", maxDistance);
#else // CONFIG_ALIRO_BLE_UWB

	ARG_UNUSED(maxDistance);
#endif // CONFIG_ALIRO_BLE_UWB
}

uint32_t AccessManagerImpl::_GetMaxAllowedDistance()
{
#ifdef CONFIG_ALIRO_BLE_UWB
	MutexGuard lock{ sMutex };

	return mMaxAllowedDistance;
#else
	return 0;
#endif // CONFIG_ALIRO_BLE_UWB
}

void AccessManagerImpl::_HandleRangingSessionStateChanged(SessionContext sessionContext, RangingSessionState state)
{
	switch (state) {
	case RangingSessionState::Ranging:
		LOG_INF("Ranging state changed to Ranging (session: %p)", sessionContext);
		break;
	case RangingSessionState::RangingSuspended:
		LOG_INF("Ranging state changed to Ranging Suspended (session: %p)", sessionContext);
		break;
	case RangingSessionState::RangingResumed:
		LOG_INF("Ranging state changed to Ranging Resumed (session: %p)", sessionContext);
		break;
	case RangingSessionState::Destroyed:
		LOG_INF("Ranging state changed to Destroyed (session: %p)", sessionContext);
		break;
	default:
		break;
	}
}

void AccessManagerImpl::_HandleRangingSessionData(SessionContext sessionContext, const UwbRangingData &uwbData)
{
	LOG_DBG("Handling ranging session data - length: %u for session: %p", uwbData.mLength, sessionContext);

#ifdef CONFIG_ALIRO_BLE_UWB
	const auto currentSessionInRange = AnalyzeUwbRangingData(uwbData);
	{
		MutexGuard lock{ sMutex };
		auto *sessionCtx = FindRangingSession(sessionContext);
		VerifyOrReturn(sessionCtx, LOG_ERR("Session context not found for handle: %p", sessionContext));
		sessionCtx->mInRange = currentSessionInRange;
#ifdef CONFIG_ALIRO_ACCESS_MANAGER_TERMINATE_SESSION_ON_TIMEOUT
		if (!sessionCtx->mRangingSessionTimer.IsRunning()) {
			sessionCtx->mRangingSessionTimer.Start();
		}
#endif // CONFIG_ALIRO_ACCESS_MANAGER_TERMINATE_SESSION_ON_TIMEOUT
	}
	const bool userDeviceInRange = IsUserDeviceInRange();

#ifdef CONFIG_ALIRO_ACCESS_MANAGER_TERMINATE_SESSION_ON_ACCESS_GRANTED
	if (mInRange && currentSessionInRange) {
		TerminateAliroSession(sessionContext);
	}
#endif // CONFIG_ALIRO_ACCESS_MANAGER_TERMINATE_SESSION_ON_ACCESS_GRANTED

	// Only detect access status changes
	VerifyOrReturn(userDeviceInRange != mInRange);
	mInRange = userDeviceInRange;

	auto readerState = mInRange ? ReaderStateByte::Unsecured : ReaderStateByte::Secured;

	Aliro::AliroStack::Instance().SendReaderStatusChangedMessage(
		OperationSource::ThisUserDeviceInBluetoothLeUwbAliroFlow, readerState);

	if (mInRange) {
		UnlockAction();
#ifdef CONFIG_ALIRO_ACCESS_MANAGER_TERMINATE_SESSION_ON_ACCESS_GRANTED
		TerminateAliroSession(sessionContext);
#endif // CONFIG_ALIRO_ACCESS_MANAGER_TERMINATE_SESSION_ON_ACCESS_GRANTED
	} else
#endif // CONFIG_ALIRO_BLE_UWB
	{
		LockAction();
	}
}

void AccessManagerImpl::_HandleSessionTermination(SessionContext sessionContext)
{
	VerifyOrReturn(sessionContext, LOG_ERR("Session context is null"));

	LOG_INF("Handling session termination");

#ifdef CONFIG_ALIRO_BLE_UWB
	RemoveRangingSession(sessionContext);

	// If any ranging session is still in range, then keep this flag true.
	mInRange = IsUserDeviceInRange();
#endif // CONFIG_ALIRO_BLE_UWB
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

	// Use only AC keys
	return IsPublicKeyStored(mAcKeys, userPublicKey, keyIndex);
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

AliroError AccessManagerImpl::_GetPublicKey(size_t keyIndex, CryptoTypes::PublicKey &publicKey)
{
	MutexGuard lock{ sMutex };
	VerifyOrReturnStatus(keyIndex < mAcKeys.mKeys.size(), ALIRO_INVALID_ARGUMENT,
			     LOG_WRN("Key index out of range"));
	VerifyOrReturnStatus(mAcKeys.mKeys[keyIndex].has_value(), ALIRO_INVALID_ARGUMENT,
			     LOG_WRN("Key not found in storage"));

	publicKey = mAcKeys.mKeys[keyIndex].value();
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

#ifdef CONFIG_ALIRO_BLE_UWB

	// Skip Reader Status Changed Message for NFC sessions when Reader is opened via BLE+UWB
	if (!mInRange) {
		Aliro::AliroStack::Instance().SendReaderStatusChangedMessage(OperationSource::ThisUserDeviceInNfc,
									     ReaderStateByte::Unsecured);
	}

	// For NFC sessions with UWB enabled, only unlock if not already in range
	// This prevents double-unlocking when user is already detected via UWB
	return !mInRange;

#else // CONFIG_ALIRO_BLE_UWB

	// For NFC sessions without UWB, always unlock immediately
	return true;

#endif // CONFIG_ALIRO_BLE_UWB
}

#ifdef CONFIG_ALIRO_BLE_UWB
bool AccessManagerImpl::AnalyzeUwbRangingData(const UwbRangingData &uwbData)
{
	LOG_DBG("Analyzing UWB ranging data - length: %u", uwbData.mLength);

	// Check if UWB data is valid
	VerifyOrReturnFalse(uwbData.mData && uwbData.mLength, LOG_WRN("Invalid UWB ranging data"));

	// Extract distance from UWB data
	auto distance = ExtractDistanceFromUwbData(uwbData);

	VerifyOrReturnFalse(distance, LOG_WRN("Failed to extract distance from UWB data"));

	MutexGuard lock{ sMutex };

	LOG_DBG("Extracted distance: %u cm, max allowed: %u cm", distance.value(), mMaxAllowedDistance);

	// Check if distance is within acceptable range
	VerifyOrReturnFalse(distance.value() <= mMaxAllowedDistance,
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
						const SessionContext sessionCtx)
{
	auto *newCtx = Aliro::new_nothrow<RangingSessionContext>(
#ifdef CONFIG_ALIRO_ACCESS_MANAGER_TERMINATE_SESSION_ON_TIMEOUT
		CONFIG_ALIRO_ACCESS_MANAGER_SESSION_TIMEOUT_MS,
		[](Timer::Context ctx) { AccessManagerImpl::Instance().TerminateAliroSession(ctx); },
		const_cast<Timer::Context>(sessionCtx)
#endif // CONFIG_ALIRO_ACCESS_MANAGER_TERMINATE_SESSION_ON_TIMEOUT
	);
	VerifyOrReturnStatus(newCtx, ALIRO_NO_MEMORY, LOG_ERR("Cannot allocate context for UWB session."));

	newCtx->mInRange = false;
	newCtx->mSessionContext = sessionCtx;

	MutexGuard lock{ sMutex };

	AliroError status =
		Uwb::UltraWideBandImpl::Instance().ConfigureRangingSession(rangingSessionId, ursk, sessionCtx);
	VerifyOrExit(status == ALIRO_NO_ERROR, LOG_ERR("Failed to configure ranging session: %d", status.ToInt()));

	sys_slist_append(&mActiveSessions, &newCtx->mNode);

	return ALIRO_NO_ERROR;

exit:
#ifdef CONFIG_ALIRO_ACCESS_MANAGER_TERMINATE_SESSION_ON_TIMEOUT
	newCtx->mRangingSessionTimer.Stop();
#endif // CONFIG_ALIRO_ACCESS_MANAGER_TERMINATE_SESSION_ON_TIMEOUT
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
#ifdef CONFIG_ALIRO_ACCESS_MANAGER_TERMINATE_SESSION_ON_TIMEOUT
			ctx->mRangingSessionTimer.Stop();
#endif // CONFIG_ALIRO_ACCESS_MANAGER_TERMINATE_SESSION_ON_TIMEOUT
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

bool AccessManagerImpl::IsUserDeviceInRange()
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

void AccessManagerImpl::TerminateAliroSession(SessionContext sessionContext)
{
	LOG_DBG("Terminating Aliro session for context: %p", sessionContext);
	VerifyAndCall(mStackCallbacks.mTerminateSessionClb, sessionContext);
}
#endif // CONFIG_ALIRO_BLE_UWB

template <size_t T>
AliroError AccessManagerImpl::RemoveKeyFromContainer(StoredKeys<T> &container, const CryptoTypes::PublicKey &publicKey,
						     size_t &keyIndex) const
{
	MutexGuard lock{ sMutex };

	for (size_t id = 0; id < container.mKeys.size(); id++) {
		if (container.mKeys[id] == publicKey) {
			container.mKeys[id].reset();
			container.mCount--;
			keyIndex = id;

			LOG_INF("Removed public key from container. Total keys: %u", container.mCount);
			return ALIRO_NO_ERROR;
		}
	}

	LOG_WRN("Public key not found in storage");
	return ALIRO_PUBLIC_KEY_NOT_FOUND;
}

template <size_t T>
AliroError AccessManagerImpl::AddKeyToContainer(StoredKeys<T> &container, const CryptoTypes::PublicKey &publicKey,
						std::optional<size_t> keyIndex) const
{
	MutexGuard lock{ sMutex };

	// Check if key already exists, if so, just do nothing
	VerifyOrReturnStatus(!IsPublicKeyStored(container, publicKey), ALIRO_NO_ERROR,
			     LOG_WRN("Public key already exists in storage"));

	// Check if storage is full
	VerifyOrReturnStatus(container.mCount < container.mKeys.size(), ALIRO_NO_MEMORY,
			     LOG_WRN("Cannot add public key - storage is full (%u/%u)", container.mCount,
				     container.mKeys.size()));

	size_t insertIndex{ 0 };
	if (!keyIndex.has_value()) {
		// The keyIndex is not provided, so we need to find the first empty slot in the storage
		for (size_t id = 0; id < container.mKeys.size(); id++) {
			if (!container.mKeys[id].has_value()) {
				insertIndex = id;
				break;
			}
		}
	} else {
		insertIndex = keyIndex.value();
	}

	VerifyOrReturnStatus(insertIndex < container.mKeys.size(), ALIRO_NO_MEMORY,
			     LOG_WRN("No empty slot found in storage"));
	// Add the key to storage
	container.mKeys[insertIndex] = publicKey;
	container.mCount++;

	LOG_INF("Added public key to slot %u. Total keys: %u", insertIndex, container.mCount);

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

} // namespace Aliro
