
/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include "access_manager/access_manager.h"

#ifdef CONFIG_ALIRO_ACCESS_MANAGER_TERMINATE_SESSION_ON_TIMEOUT
#include "aliro/timer.h"
#endif // CONFIG_ALIRO_ACCESS_MANAGER_TERMINATE_SESSION_ON_TIMEOUT

#include <array>
#include <cstring>
#include <optional>

#include <zephyr/kernel.h>

namespace Aliro {

class AccessManagerImpl final : public AccessManager {
private:
	friend class AccessManager;
	friend AccessManager &AccessManagerInstance();

	/**
	 * @brief Get an instance of the the AccessManagerImpl.
	 *
	 * @return Reference to the AccessManagerImpl instance.
	 */
	static AccessManagerImpl &Instance()
	{
		static AccessManagerImpl sInstance;
		return sInstance;
	}

	/**
	 * @brief Set the application callbacks for the AccessManager.
	 *
	 * @param callbacks Application callbacks.
	 */
	void _SetApplicationCallbacks(const ApplicationCallbacks &callbacks);

	/**
	 * @brief Set the stack callbacks.
	 *
	 * @param callbacks Stack callbacks.
	 */
	void _SetStackCallbacks(const StackCallbacks &callbacks);

	/**
	 * @brief Verifies the access credential based on provided inputs.
	 *
	 * @param userPublicKey The User Device public key to verify.
	 * @param isNfcSession Indicates if the session is a NFC session.
	 * @param sessionContext A pointer to the session context.
	 *
	 * @return ALIRO_NO_ERROR on success, error code otherwise.
	 */
	AliroError _VerifyAccessCredential(const CryptoTypes::PublicKey &userPublicKey, bool isNfcSession,
					   SessionContext sessionContext);

#ifdef CONFIG_ALIRO_BLE_UWB
	/**
	 * @brief Starts a ranging session based on provided inputs.
	 *
	 * @param rangingSessionId The ranging session ID.
	 * @param ursk The ranging session key.
	 * @param sessionContext A pointer to the session context.
	 *
	 * @return ALIRO_NO_ERROR on success, error code otherwise.
	 */
	AliroError _StartRangingSession(uint32_t rangingSessionId, const CryptoTypes::Ursk &ursk,
					SessionContext sessionContext);
#endif // CONFIG_ALIRO_BLE_UWB

	/**
	 * @brief Add a new public key to the AccessManager.
	 *
	 * @param publicKey The public key to add.
	 *
	 * @return ALIRO_NO_ERROR on success, error code on failure.
	 */
	AliroError _AddPublicKey(const CryptoTypes::PublicKey &publicKey);

	/**
	 * @brief Remove a public key from the AccessManager.
	 *
	 * @param publicKey The public key to remove.
	 *
	 * @return ALIRO_NO_ERROR on success, error code on failure.
	 */
	AliroError _RemovePublicKey(const CryptoTypes::PublicKey &publicKey);

	/**
	 * @brief Clear all stored public keys.
	 */
	void _ClearStoredKeys();

	/**
	 * @brief Set the maximum allowed distance for UWB ranging (in centimeters).
	 *
	 * @param maxDistance Maximum distance in centimeters.
	 */
	void _SetMaxAllowedDistance(uint32_t maxDistance);

	/**
	 * @brief Get the maximum allowed distance for UWB ranging.
	 *
	 * @return Maximum distance in centimeters.
	 */
	uint32_t _GetMaxAllowedDistance();

	/**
	 * @brief Handles the ranging session data.
	 *
	 * @param sessionContext The session context.
	 * @param uwbData The ranging session data.
	 */
	void _HandleRangingSessionData(SessionContext sessionContext, const UwbRangingData &uwbData);

	/**
	 * @brief Handles the session termination.
	 *
	 * @param sessionContext The session context.
	 */
	void _HandleSessionTermination(SessionContext sessionContext);

private:
	AccessManagerImpl() = default;
	~AccessManagerImpl() = default;

	AccessManagerImpl(const AccessManagerImpl &) = delete;
	AccessManagerImpl &operator=(const AccessManagerImpl &) = delete;
	AccessManagerImpl(AccessManagerImpl &&) = delete;
	AccessManagerImpl &operator=(AccessManagerImpl &&) = delete;

	/**
	 * @brief Signals the access granted action.
	 *
	 * @param isNfcSession Indicates if the session is a NFC session.
	 *
	 * This callback is called when the User Device is authenticated and the access is granted.
	 */
	void AccessGrantedAction(bool isNfcSession) const;

	/**
	 * @brief Signals the access denied action.
	 *
	 * @param isNfcSession Indicates if the session is a NFC session.
	 *
	 * This callback is called when the User Device is not authenticated or the access is denied.
	 */
	void AccessDeniedAction(bool isNfcSession) const;

	/**
	 * @brief Signals the unlock action.
	 *
	 * This callback is called when access is granted and the door should be unlocked.
	 * In case of:
	 * - UWB ranging, this callback is called when the User Device is in range.
	 * - NFC, this callback is called just after the access credential is verified.
	 */
	void UnlockAction() const;

	/**
	 * @brief Signals the lock action.
	 *
	 * This callback is called when the door should be locked.
	 * In case of:
	 * - UWB ranging, this callback is called when the User Device is out of range or BLE session is terminated.
	 */
	void LockAction() const;

	bool VerifyPublicKey(const CryptoTypes::PublicKey &userPublicKey);
	bool IsPublicKeyStored(const CryptoTypes::PublicKey &userPublicKey);
	bool ShouldUnlockImmediately(bool isNfcSession) const;

#ifdef CONFIG_ALIRO_BLE_UWB
	struct RangingSessionContext {
		sys_snode_t mNode{};
#ifdef CONFIG_ALIRO_ACCESS_MANAGER_TERMINATE_SESSION_ON_TIMEOUT
		RangingSessionContext(uint32_t timeoutMs, Timer::Callback callback, Timer::Context userData)
			: mRangingSessionTimer(timeoutMs, callback, userData)
		{
		}
		Timer mRangingSessionTimer;
#endif // CONFIG_ALIRO_ACCESS_MANAGER_TERMINATE_SESSION_ON_TIMEOUT
		SessionContext mSessionContext{};
		bool mInRange{ false };
	};

	bool AnalyzeUwbRangingData(const UwbRangingData &uwbData);
	std::optional<uint16_t> ExtractDistanceFromUwbData(const UwbRangingData &uwbData) const;
	AliroError AddRangingSession(uint32_t rangingSessionId, const CryptoTypes::Ursk &ursk,
				     const SessionContext sessionCtx);
	void RemoveRangingSession(SessionContext sessionCtx);
	RangingSessionContext *FindRangingSession(const SessionContext sessionCtx);
	bool IsUserDeviceInRange();
	void TerminateAliroSession(SessionContext sessionContext);

	static constexpr uint16_t kDefaultMaxAllowedDistance{ CONFIG_ALIRO_ACCESS_MANAGER_MAX_ALLOWED_DISTANCE_CM };

	// Maximum allowed distance for UWB ranging (in centimeters)
	uint32_t mMaxAllowedDistance{ kDefaultMaxAllowedDistance };
	// Session context for the current ranging session.
	bool mInRange{ false };
	sys_slist_t mActiveSessions{};
#endif // CONFIG_ALIRO_BLE_UWB

	ApplicationCallbacks mCallbacks{};
	StackCallbacks mStackCallbacks{};
	static constexpr size_t kMaxStoredKeys{ CONFIG_ALIRO_ACCESS_MANAGER_MAX_STORED_KEYS };
	// Storage for authorized public keys
	std::array<CryptoTypes::PublicKey, kMaxStoredKeys> mStoredKeys{};
	// Number of keys currently stored
	size_t mStoredKeyCount{ 0 };
	k_mutex mMutex{};
};

/**
 * @brief Get the singleton instance of the AccessManagerImpl.
 *
 * @return The instance of the AccessManagerImpl.
 */
inline AccessManager &AccessManagerInstance()
{
	return AccessManagerImpl::Instance();
}

} // namespace Aliro
