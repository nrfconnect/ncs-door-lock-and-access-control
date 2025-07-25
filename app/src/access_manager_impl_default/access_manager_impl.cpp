/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "access_manager_impl.h"
#include "aliro/mutex_guard.h"
#include "aliro/utils.h"

#ifdef CONFIG_ALIRO_BLE_TP
#include "uwb_impl.h"
#endif // CONFIG_ALIRO_BLE_TP

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/byteorder.h>

#include <cstdint>
#include <optional>

LOG_MODULE_REGISTER(access_manager, CONFIG_NCS_DOOR_LOCK_APP_LOG_LEVEL);

#ifdef CONFIG_AUTOMATIC_RANGING_SESSION_INITIATION
namespace {
constexpr uint16_t kRangingStartTimeoutMs = CONFIG_RANGING_START_TIMEOUT_MS;
} // namespace
#endif // CONFIG_AUTOMATIC_RANGING_SESSION_INITIATION

namespace Aliro {

AliroError AccessManagerImpl::_Init(const Callbacks &callbacks)
{
	mCallbacks = callbacks;

#ifdef CONFIG_AUTOMATIC_RANGING_SESSION_INITIATION
	k_timer_init(
		&mRangingStartTimer,
		[](k_timer *timer) {
			auto accessMgr = static_cast<AccessManagerImpl *>(k_timer_user_data_get(timer));
			VerifyOrDie(accessMgr, "Invalid timer context");
			k_work_submit(&accessMgr->mWork);
		},
		nullptr);

	k_work_init(&mWork, [](k_work *work) {
		LOG_DBG("Ranging start timer expired");
		// TODO: Workaround for one session.

		const AccessManagerImpl *accessMgr = CONTAINER_OF(work, AccessManagerImpl, mWork);
		AliroError error =
			Uwb::UltraWideBandImpl::Instance().InitiateRangingSession(accessMgr->mSessionContext);
		VerifyOrReturn(error == ALIRO_NO_ERROR,
			       LOG_ERR("Failed to initiate ranging session: %d", error.ToInt()));
	});

	k_timer_user_data_set(&mRangingStartTimer, this);

#endif // CONFIG_AUTOMATIC_RANGING_SESSION_INITIATION

	LOG_INF("AccessManager initialized");

	return ALIRO_NO_ERROR;
}

AliroError AccessManagerImpl::_StartAccessDecision(const CryptoTypes::PublicKey &userPublicKey,
						   SessionContext sessionContext)
{
	{
		MutexGuard lock{ mMutex };

		// Verify if a public key is present in the Reader's database (whether the User Device is a trusted one)
		VerifyOrReturnStatus(VerifyPublicKey(userPublicKey), ALIRO_INVALID_ARGUMENT,
				     LOG_INF("Provided Under Device public key not found in Access Manager database"););
	}

	AccessGrantedAction();

	return ALIRO_NO_ERROR;
}

#ifdef CONFIG_ALIRO_BLE_TP
AliroError AccessManagerImpl::_StartAccessDecision(const CryptoTypes::PublicKey &userPublicKey,
						   uint32_t rangingSessionId, const CryptoTypes::Ursk &ursk,
						   SessionContext sessionContext)
{
#ifdef CONFIG_AUTOMATIC_RANGING_SESSION_INITIATION
	k_timer_stop(&mRangingStartTimer);
	k_work_sync sync{};
	k_work_cancel_sync(&mWork, &sync);
#endif // CONFIG_AUTOMATIC_RANGING_SESSION_INITIATION

	{
		MutexGuard lock{ mMutex };

		// Verify if a public key is present in the Reader's database (whether the User Device is a trusted one)
		VerifyOrReturnStatus(VerifyPublicKey(userPublicKey), ALIRO_INVALID_ARGUMENT,
				     LOG_INF("Provided Under Device public key not found in Access Manager database"););

		VerifyOrReturnStatus(mSessionContext == nullptr, ALIRO_INVALID_STATE,
				     LOG_ERR("Ranging session already started"));

		AliroError status = Uwb::UltraWideBandImpl::Instance().ConfigureRangingSession(rangingSessionId, ursk,
											       sessionContext);
		VerifyOrReturnStatus(status == ALIRO_NO_ERROR, status,
				     LOG_ERR("Failed to configure ranging session: %d", status.ToInt()));

		mSessionContext = sessionContext;
	}

#ifdef CONFIG_AUTOMATIC_RANGING_SESSION_INITIATION
	k_timer_start(&mRangingStartTimer, K_MSEC(kRangingStartTimeoutMs), K_NO_WAIT);
#endif // CONFIG_AUTOMATIC_RANGING_SESSION_INITIATION

	return ALIRO_NO_ERROR;
}

#endif // CONFIG_ALIRO_BLE_TP

AliroError AccessManagerImpl::_AddPublicKey(const CryptoTypes::PublicKey &publicKey)
{
	MutexGuard lock{ mMutex };

	// Check if key already exists, if so, just do nothing
	VerifyOrReturnStatus(!IsPublicKeyStored(publicKey), ALIRO_NO_ERROR,
			     LOG_WRN("Public key already exists in storage"));

	// Check if storage is full
	VerifyOrReturnStatus(mStoredKeyCount < kMaxStoredKeys, ALIRO_NO_MEMORY,
			     LOG_WRN("Cannot add public key - storage is full (%zu/%zu)", mStoredKeyCount,
				     kMaxStoredKeys));
	// Add the key to storage
	mStoredKeys[mStoredKeyCount] = publicKey;
	mStoredKeyCount++;

	LOG_INF("Added public key to storage. Total keys: %zu", mStoredKeyCount);

	return ALIRO_NO_ERROR;
}

AliroError AccessManagerImpl::_RemovePublicKey(const CryptoTypes::PublicKey &publicKey)
{
	MutexGuard lock{ mMutex };

	for (size_t i = 0; i < mStoredKeyCount; ++i) {
		if (mStoredKeys[i] == publicKey) {
			// Move the last key to this position instead of moving the entire array and decrement
			// count
			if (i < mStoredKeyCount - 1) {
				mStoredKeys[i] = mStoredKeys[mStoredKeyCount - 1];
			}
			mStoredKeyCount--;
			LOG_INF("Removed public key from storage. Total keys: %zu", mStoredKeyCount);
			return ALIRO_NO_ERROR;
		}
	}

	LOG_WRN("Public key not found in storage");
	return ALIRO_INVALID_ARGUMENT;
}

void AccessManagerImpl::_ClearStoredKeys()
{
	MutexGuard lock{ mMutex };

	mStoredKeyCount = 0;

	LOG_INF("Cleared all stored public keys");
}

void AccessManagerImpl::_SetMaxAllowedDistance(uint32_t maxDistance)
{
#ifdef CONFIG_ALIRO_BLE_TP
	MutexGuard lock{ mMutex };

	mMaxAllowedDistance = maxDistance;
	LOG_INF("Set maximum allowed distance to %u cm", maxDistance);
#endif // CONFIG_ALIRO_BLE_TP
}

uint32_t AccessManagerImpl::_GetMaxAllowedDistance()
{
#ifdef CONFIG_ALIRO_BLE_TP
	MutexGuard lock{ mMutex };

	return mMaxAllowedDistance;
#else
	return 0;
#endif // CONFIG_ALIRO_BLE_TP
}

void AccessManagerImpl::_HandleRangingSessionData(SessionContext sessionContext, const UwbRangingData &uwbData)
{
	LOG_DBG("Handling ranging session data - length: %zu for session: %p", uwbData.mLength, sessionContext);

#ifdef CONFIG_AUTOMATIC_RANGING_SESSION_INITIATION
	k_timer_stop(&mRangingStartTimer);
	k_work_sync sync{};
	k_work_cancel_sync(&mWork, &sync);
#endif // CONFIG_AUTOMATIC_RANGING_SESSION_INITIATION

#ifdef CONFIG_ALIRO_BLE_TP
	const bool inRange = AnalyzeUwbRangingData(uwbData);
	if (inRange) {
		AccessGrantedAction();
	} else
#endif
	{
		AccessDeniedAction();
	}
}

void AccessManagerImpl::_HandleSessionTermination(SessionContext sessionContext)
{
	VerifyOrReturn(sessionContext, LOG_ERR("Session context is null"));

	LOG_INF("Handling session termination");

#ifdef CONFIG_ALIRO_BLE_TP
	MutexGuard lock{ mMutex };

	if (mSessionContext == sessionContext) {
		mSessionContext = nullptr;

		AliroError status = Uwb::UltraWideBandImpl::Instance().TerminateRangingSession(sessionContext);
		VerifyOrReturn(status == ALIRO_NO_ERROR || status == ALIRO_ERROR_NOT_IMPLEMENTED,
			       LOG_ERR("Cannot terminate UWB ranging session: %d", status.ToInt()));
	}
#endif // CONFIG_ALIRO_BLE_TP
}

bool AccessManagerImpl::VerifyPublicKey(const CryptoTypes::PublicKey &userPublicKey)
{
	LOG_DBG("Verifying public key against %zu stored keys", mStoredKeyCount);

	// Check if the user's public key matches any stored key
	VerifyOrReturnFalse(IsPublicKeyStored(userPublicKey), LOG_DBG("No matching public key found"));

	return true;
}

bool AccessManagerImpl::IsPublicKeyStored(const CryptoTypes::PublicKey &userPublicKey)
{
	for (size_t i = 0; i < mStoredKeyCount; ++i) {
		if (mStoredKeys[i] == userPublicKey) {
			return true;
		}
	}
	return false;
}

void AccessManagerImpl::AccessGrantedAction() const
{
	LOG_INF("ACCESS GRANTED");
	VerifyAndCall(mCallbacks.mAccessGrantedIndicatorClb);
}

void AccessManagerImpl::AccessDeniedAction() const
{
	LOG_INF("ACCESS DENIED");
}

#ifdef CONFIG_ALIRO_BLE_TP
bool AccessManagerImpl::AnalyzeUwbRangingData(const UwbRangingData &uwbData)
{
	LOG_DBG("Analyzing UWB ranging data - length: %zu", uwbData.mLength);

	// Check if UWB data is valid
	VerifyOrReturnFalse(uwbData.mData && uwbData.mLength, LOG_WRN("Invalid UWB ranging data"));

	// Extract distance from UWB data
	auto distance = ExtractDistanceFromUwbData(uwbData);

	VerifyOrReturnFalse(distance, LOG_WRN("Failed to extract distance from UWB data"));

	MutexGuard lock{ mMutex };

	LOG_DBG("Extracted distance: %u cm, max allowed: %u cm", distance.value(), mMaxAllowedDistance);

	// Check if distance is within acceptable range
	VerifyOrReturnFalse(distance.value() <= mMaxAllowedDistance,
			    LOG_WRN("Distance check failed - user device is too far"));

	LOG_DBG("Distance check passed, user device is within range");
	return true;
}

std::optional<uint16_t> AccessManagerImpl::ExtractDistanceFromUwbData(const UwbRangingData &uwbData) const
{
	// TODO: Implement the actual UWB ranging data format
	// For now, we'll assume the distance is in the first 2 bytes of the UWB data
	VerifyOrReturnValue(uwbData.mLength == sizeof(uint16_t), std::nullopt, LOG_ERR("Invalid UWB data length"));
	return std::make_optional<uint16_t>(sys_get_be16(uwbData.mData));
}
#endif // CONFIG_ALIRO_BLE_TP

} // namespace Aliro
