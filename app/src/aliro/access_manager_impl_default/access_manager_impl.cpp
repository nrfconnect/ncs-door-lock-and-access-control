/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "access_manager_impl.h"
#include "aliro/mutex_guard.h"
#include "aliro/utils.h"

#ifdef CONFIG_ALIRO_BLE_TP
#include "aliro/memory.h"
#include "uwb_impl.h"
#endif // CONFIG_ALIRO_BLE_TP

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/byteorder.h>

#include <cstdint>
#include <optional>

LOG_MODULE_REGISTER(access_manager, CONFIG_NCS_DOOR_LOCK_APP_LOG_LEVEL);

namespace Aliro {

AliroError AccessManagerImpl::_Init(const ApplicationCallbacks &callbacks)
{
	mCallbacks = callbacks;

	LOG_INF("AccessManager initialized");

	return ALIRO_NO_ERROR;
}

void AccessManagerImpl::_SetStackCallbacks(const StackCallbacks &callbacks)
{
	mStackCallbacks = callbacks;
}

AliroError AccessManagerImpl::_VerifyAccessCredential(const CryptoTypes::PublicKey &userPublicKey, bool isNfcSession,
						      SessionContext sessionContext)
{
	{
		MutexGuard lock{ mMutex };

		// Verify if a public key is present in the Reader's database (whether the User Device is a trusted one)
		VerifyOrReturnStatus(VerifyPublicKey(userPublicKey), ALIRO_INVALID_ARGUMENT,
				     LOG_INF("Provided User Device public key not found in Access Manager database"););
	}

	if (isNfcSession) {
		AccessGrantedAction();
	}

	return ALIRO_NO_ERROR;
}

#ifdef CONFIG_ALIRO_BLE_TP
AliroError AccessManagerImpl::_StartRangingSession(uint32_t rangingSessionId, const CryptoTypes::Ursk &ursk,
						   SessionContext sessionContext)
{
	MutexGuard lock{ mMutex };

	AliroError status =
		Uwb::UltraWideBandImpl::Instance().ConfigureRangingSession(rangingSessionId, ursk, sessionContext);
	VerifyOrReturnStatus(status == ALIRO_NO_ERROR, status,
			     LOG_ERR("Failed to configure ranging session: %d", status.ToInt()));

	return AddRangingSession(sessionContext);
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
			     LOG_WRN("Cannot add public key - storage is full (%u/%u)", mStoredKeyCount,
				     kMaxStoredKeys));
	// Add the key to storage
	mStoredKeys[mStoredKeyCount] = publicKey;
	mStoredKeyCount++;

	LOG_INF("Added public key to storage. Total keys: %u", mStoredKeyCount);

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
			LOG_INF("Removed public key from storage. Total keys: %u", mStoredKeyCount);
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
	LOG_DBG("Handling ranging session data - length: %u for session: %p", uwbData.mLength, sessionContext);

#ifdef CONFIG_ALIRO_BLE_TP
	const auto currentSessionInRange = AnalyzeUwbRangingData(uwbData);
	{
		MutexGuard lock{ mMutex };
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
	if (mInRange) {
		AccessGrantedAction();
#ifdef CONFIG_ALIRO_ACCESS_MANAGER_TERMINATE_SESSION_ON_ACCESS_GRANTED
		TerminateAliroSession(sessionContext);
#endif // CONFIG_ALIRO_ACCESS_MANAGER_TERMINATE_SESSION_ON_ACCESS_GRANTED
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

	AliroError status = Uwb::UltraWideBandImpl::Instance().TerminateRangingSession(sessionContext);
	VerifyOrReturn(status == ALIRO_NO_ERROR || status == ALIRO_ERROR_NOT_IMPLEMENTED,
		       LOG_ERR("Cannot terminate UWB ranging session: %d", status.ToInt()));

	RemoveRangingSession(sessionContext);

#endif // CONFIG_ALIRO_BLE_TP
}

bool AccessManagerImpl::VerifyPublicKey(const CryptoTypes::PublicKey &userPublicKey)
{
	LOG_DBG("Verifying public key against %u stored keys", mStoredKeyCount);

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
	LOG_DBG("Analyzing UWB ranging data - length: %u", uwbData.mLength);

	// Check if UWB data is valid
	VerifyOrReturnFalse(uwbData.mData && uwbData.mLength, LOG_WRN("Invalid UWB ranging data"));

	// Extract distance from UWB data
	auto distance = ExtractDistanceFromUwbData(uwbData);

	VerifyOrReturnFalse(distance, LOG_WRN("Failed to extract distance from UWB data"));

	MutexGuard lock{ mMutex };

	LOG_DBG("Extracted distance: %u cm, max allowed: %u cm", distance.value(), mMaxAllowedDistance);

	// Check if distance is within acceptable range
	VerifyOrReturnFalse(distance.value() <= mMaxAllowedDistance,
			    LOG_DBG("Distance check failed - user device is too far"));

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

AliroError AccessManagerImpl::AddRangingSession(const SessionContext sessionCtx)
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

	MutexGuard lock{ mMutex };
	sys_slist_append(&mActiveSessions, &newCtx->mNode);

	return ALIRO_NO_ERROR;
}

void AccessManagerImpl::RemoveRangingSession(SessionContext sessionCtx)
{
	MutexGuard lock{ mMutex };

	sys_snode_t *node{ nullptr };
	sys_snode_t *nodeSafe{ nullptr };
	sys_snode_t *prevNode{ nullptr };

	SYS_SLIST_FOR_EACH_NODE_SAFE (&mActiveSessions, node, nodeSafe) {
		RangingSessionContext *ctx = CONTAINER_OF(node, RangingSessionContext, mSessionContext);
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

	MutexGuard lock{ mMutex };
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

	MutexGuard lock{ mMutex };
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

#endif // CONFIG_ALIRO_BLE_TP

} // namespace Aliro
