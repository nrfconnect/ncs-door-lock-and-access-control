/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

#pragma once

#include "aliro/errors.h"
#include "aliro/types.h"

namespace Aliro {

class AccessManagerImpl;

/**
 * @brief AccessManager An interface for making access control decisions.
 *
 * This class is an API that should manage User Device public keys and makes access decisions based on:
 * - Signature verification status
 * - Public key validation against stored keys
 * - UWB ranging data analysis
 */
class AccessManager {
public:
	using SessionContext = const void *;
	using LockIndicatorCallback = void (*)();
	using AccessIndicatorCallback = void (*)(bool isAccessGranted, bool isNfcSession);
	using TerminateSessionCallback = void (*)(SessionContext sessionContext);

	/**
	 * @brief Application callbacks.
	 */
	struct ApplicationCallbacks {
		/**
		 * @brief Callback for signaling unlock action.
		 *
		 * This callback is called when access is granted and the door should be unlocked.
		 */
		LockIndicatorCallback mUnlockIndicatorClb{ nullptr };

		/**
		 * @brief Callback for signaling lock action.
		 *
		 * This callback is called when access is granted and the door should be locked.
		 */
		LockIndicatorCallback mLockIndicatorClb{ nullptr };

		/**
		 * @brief Callback for signaling access action.
		 *
		 * This callback is called when the access is granted or denied.
		 * This callback can be used for security logs.
		 */
		AccessIndicatorCallback mAccessIndicatorClb{ nullptr };
	};

	/**
	 * @brief Stack callbacks.
	 *
	 */
	struct StackCallbacks {
		/**
		 * @brief Callback for terminating the Aliro session.
		 *
		 * This callback is called when the session should be terminated.
		 */
		TerminateSessionCallback mTerminateSessionClb{ nullptr };
	};

	/**
	 * @brief Set the application callbacks for the AccessManager.
	 *
	 * @param callbacks Callbacks.
	 */
	void SetApplicationCallbacks(const ApplicationCallbacks &callbacks);

	/**
	 * @brief Set the stack callbacks.
	 *
	 * @param callbacks Stack callbacks.
	 */
	void SetStackCallbacks(const StackCallbacks &callbacks);

	/**
	 * @brief Verifies the access credential based on provided inputs.
	 *
	 * @param userPublicKey The User Device public key to verify.
	 * @param isNfcSession Indicates if the session is a NFC session.
	 * @param sessionContext A pointer to the session context.
	 *
	 * @return ALIRO_NO_ERROR on success, error code otherwise.
	 */
	AliroError VerifyAccessCredential(const CryptoTypes::PublicKey &userPublicKey, bool isNfcSession,
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
	AliroError StartRangingSession(uint32_t rangingSessionId, const CryptoTypes::Ursk &ursk,
				       SessionContext sessionContext);
#endif // CONFIG_ALIRO_BLE_UWB

	/**
	 * @brief Add a new public key to the AccessManager.
	 *
	 * @param publicKey The User Device public key to add.
	 *
	 * @return ALIRO_NO_ERROR on success, error code on failure.
	 */
	AliroError AddPublicKey(const CryptoTypes::PublicKey &publicKey);

	/**
	 * @brief Remove a public key from the AccessManager.
	 *
	 * @param publicKey The User Device public key to remove.
	 *
	 * @return ALIRO_NO_ERROR on success, error code on failure.
	 */
	AliroError RemovePublicKey(const CryptoTypes::PublicKey &publicKey);

	/**
	 * @brief Clear all stored public keys.
	 */
	void ClearStoredKeys();

	/**
	 * @brief Set the maximum allowed distance for UWB ranging (in centimeters).
	 *
	 * @param maxDistance Maximum distance in centimeters.
	 */
	void SetMaxAllowedDistance(uint32_t maxDistance);

	/**
	 * @brief Get the maximum allowed distance for UWB ranging.
	 *
	 * @return Maximum distance in centimeters.
	 */
	uint32_t GetMaxAllowedDistance();

	/**
	 * @brief Handles the ranging session data.
	 *
	 * @param sessionContext The session context.
	 * @param uwbData The ranging session data.
	 */
	void HandleRangingSessionData(SessionContext sessionContext, const UwbRangingData &uwbData);

	/**
	 * @brief Handles the session termination.
	 *
	 * @param sessionContext The session context.
	 */
	void HandleSessionTermination(SessionContext sessionContext);

private:
	AccessManagerImpl *Impl();
	const AccessManagerImpl *Impl() const;
};

/**
 * @brief Get the singleton instance of the implementation of the AccessManager.
 *
 * @return Reference to the singleton instance.
 */
extern AccessManager &AccessManagerInstance();

} // namespace Aliro

#include "access_manager_impl.h"
