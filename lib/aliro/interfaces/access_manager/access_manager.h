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
 * This class is an API that should manage user device public keys and makes access decisions based on:
 * - Signature verification status
 * - Public key validation against stored keys
 * - UWB ranging data analysis
 */
class AccessManager {
public:
	using AccessGrantedIndicatorCallback = void (*)();

	/**
	 * @brief Callbacks for the command handlers.
	 */
	struct Callbacks {
		/**
		 * @brief Callback for signaling access granted.
		 *
		 * This callback is called when access is granted.
		 */
		AccessGrantedIndicatorCallback mAccessGrantedIndicatorClb{ nullptr };
	};

	/**
	 * @brief Initialize the AccessManager.
	 *
	 * @param callbacks Callbacks.
	 *
	 * @return ALIRO_NO_ERROR on success, error code otherwise.
	 */
	AliroError Init(const Callbacks &callbacks);

	/**
	 * @brief Starts an access decision process based on provided inputs.
	 *
	 * @param userPublicKey The user device public key to verify.
	 * @param isBleSession Indicates if the access decision is being made in the BLE transport context.
	 *
	 * @return ALIRO_NO_ERROR on success, error code otherwise.
	 */
	AliroError StartAccessDecision(const CryptoTypes::PublicKey &userPublicKey, bool isBleSession);

	/**
	 * @brief Add a new public key to the AccessManager.
	 *
	 * @param publicKey The public key to add.
	 *
	 * @return ALIRO_NO_ERROR on success, error code on failure.
	 */
	AliroError AddPublicKey(const CryptoTypes::PublicKey &publicKey);

	/**
	 * @brief Remove a public key from the AccessManager.
	 *
	 * @param publicKey The public key to remove.
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
	 * @param uwbData The ranging session data.
	 */
	void HandleRangingSessionData(const UwbRangingData &uwbData);

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
