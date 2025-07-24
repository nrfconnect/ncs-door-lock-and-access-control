
/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include "access_manager/access_manager.h"

#include <array>
#include <cstring>
#include <optional>

#include <zephyr/kernel.h>

namespace Aliro {

class AccessManagerImpl : public AccessManager {
private:
	friend AccessManager &AccessManagerInstance();
	friend class AccessManager;

	/**
	 * @brief Initialize the AccessManager.
	 *
	 * @param callbacks Callbacks.
	 *
	 * @return ALIRO_NO_ERROR on success, error code otherwise.
	 */
	AliroError _Init(const Callbacks &callbacks);

	/**
	 * @brief Starts an access decision process based on provided inputs.
	 *
	 * @param userPublicKey The user device public key to verify.
	 * @param sessionContext A pointer to the session context.
	 *
	 * @return ALIRO_NO_ERROR on success, error code otherwise.
	 */
	AliroError _StartAccessDecision(const CryptoTypes::PublicKey &userPublicKey, SessionContext sessionContext);

#ifdef CONFIG_ALIRO_BLE_TP
	/**
	 * @brief Starts an access decision process based on provided inputs.
	 *
	 * @param userPublicKey The user device public key to verify.
	 * @param rangingSessionId The ranging session ID.
	 * @param ursk The ranging session key.
	 * @param sessionContext A pointer to the session context.
	 *
	 * @return ALIRO_NO_ERROR on success, error code otherwise.
	 */
	AliroError _StartAccessDecision(const CryptoTypes::PublicKey &userPublicKey, uint32_t rangingSessionId,
					const CryptoTypes::Ursk &ursk, SessionContext sessionContext);
#endif // CONFIG_ALIRO_BLE_TP

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
	 * @param uwbData The ranging session data.
	 */
	void _HandleRangingSessionData(const UwbRangingData &uwbData);

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

	static constexpr size_t kMaxStoredKeys{ CONFIG_ACCESS_MANAGER_MAX_STORED_KEYS };
#ifdef CONFIG_ALIRO_BLE_TP
	static constexpr uint16_t kDefaultMaxAllowedDistance{ CONFIG_ACCESS_MANAGER_MAX_ALLOWED_DISTANCE_CM };
#endif // CONFIG_ALIRO_BLE_TP

	void AccessGrantedAction() const;
	void AccessDeniedAction() const;

	bool VerifyPublicKey(const CryptoTypes::PublicKey &userPublicKey);
	bool IsPublicKeyStored(const CryptoTypes::PublicKey &userPublicKey);

#ifdef CONFIG_ALIRO_BLE_TP
	bool AnalyzeUwbRangingData(const UwbRangingData &uwbData);
	std::optional<uint16_t> ExtractDistanceFromUwbData(const UwbRangingData &uwbData) const;
#endif // CONFIG_ALIRO_BLE_TP

	// Storage for authorized public keys
	std::array<CryptoTypes::PublicKey, kMaxStoredKeys> mStoredKeys{};
	// Number of keys currently stored
	size_t mStoredKeyCount{ 0 };

#ifdef CONFIG_ALIRO_BLE_TP
	// Maximum allowed distance for UWB ranging (in centimeters)
	uint32_t mMaxAllowedDistance{ kDefaultMaxAllowedDistance };
	// Session context for the current ranging session.
	SessionContext mSessionContext;
#endif // CONFIG_ALIRO_BLE_TP

#ifdef CONFIG_AUTOMATIC_RANGING_SESSION_INITIATION
	k_timer mRangingStartTimer{};
#endif // CONFIG_AUTOMATIC_RANGING_SESSION_INITIATION

	Callbacks mCallbacks{};
	k_work mWork{};
	k_mutex mMutex{};
};

/**
 * @brief Get the singleton instance of the AccessManagerImpl.
 *
 * @return The instance of the AccessManagerImpl.
 */
inline AccessManager &AccessManagerInstance()
{
	static AccessManagerImpl sInstance{};
	return sInstance;
}

} // namespace Aliro
