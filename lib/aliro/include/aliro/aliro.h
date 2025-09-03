/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

#pragma once

#include "aliro/errors.h"
#include "aliro/types.h"
#ifdef CONFIG_ALIRO_BLE_UWB
#include "aliro/ble_types.h"
#include "transport/ble/ble_iface.h"
#endif // CONFIG_ALIRO_BLE_UWB

#include <cstddef>

namespace Aliro {

struct AliroConfig {
	/**
	 * @brief Enable NFC transport.
	 */
	bool mEnableNfc{ true };

#ifdef CONFIG_ALIRO_BLE_UWB

	/**
	 * @brief The maximum number of BLE sessions.
	 */
	size_t mMaxBleSessions{ 1 };

	/**
	 * @brief The BLE advertising interface.
	 */
	BleInterface::BleAdvertisingIfc *mBleAdvertising{};

#endif // CONFIG_ALIRO_BLE_UWB
};

/**
 * @brief Aliro stack.
 */
class AliroStack {
public:
	/**
	 * @brief Aliro stack callbacks.
	 */
	struct Callbacks {
		/**
		 * @brief Callback for errors.
		 *
		 * This callback is called when an error occurs.
		 *
		 * @param error The error that occurred.
		 */
		void (*mOnError)(AliroError error){ nullptr };
	};

	/**
	 * @brief Gets the instance of the Aliro stack.
	 *
	 * @return The instance of the Aliro stack.
	 */
	static AliroStack &Instance()
	{
		static AliroStack sInstance;
		return sInstance;
	}

	/**
	 * @brief Initializes the Aliro stack.
	 *
	 * @param callbacks The Access callbacks.
	 * @param config The Aliro configuration.
	 *
	 * @return ALIRO_NO_ERROR if the stack was initialized successfully, an error code otherwise.
	 */
	AliroError Init(const Callbacks &callbacks, const AliroConfig &config);

	/**
	 * @brief Provision the Reader.
	 *
	 * @param privateKeyId The private key ID.
	 * @param groupResolvingKeyId The group resolving key ID.
	 * @param identifier The reader identifier.
	 *
	 * @return ALIRO_NO_ERROR if the Reader was provisioned successfully, an error code otherwise.
	 */
	AliroError Provision(CryptoTypes::KeyId privateKeyId, CryptoTypes::KeyId groupResolvingKeyId,
			     const Identifier &identifier);

	/**
	 * @brief Sets the reader identifier.
	 *
	 * @param identifier The reader identifier.
	 */
	AliroError SetReaderIdentifier(const Identifier &identifier);

	/**
	 * @brief Starts the Aliro stack.
	 *
	 * @return ALIRO_NO_ERROR if the stack was started successfully, an error code otherwise.
	 */
	AliroError Start() const;

	/**
	 * @brief Stops the Aliro stack.
	 *
	 * @return ALIRO_NO_ERROR if the stack was stopped successfully, an error code otherwise.
	 */
	AliroError Stop() const;

	/**
	 * @brief Gets the Aliro configuration.
	 *
	 * @return The Aliro configuration.
	 */
	const AliroConfig &GetConfig() const { return mConfig; }

	/**
	 * @brief Gets the Aliro library version string.
	 *
	 * @return The Aliro library version.
	 */
	static const char *GetLibraryVersion();

#ifdef CONFIG_ALIRO_BLE_UWB

	/**
	 * @brief Sets the BLE notification which will be advertised.
	 *
	 * @param notification The BLE notification.
	 *
	 * @return ALIRO_NO_ERROR if the notification was set successfully, an error code otherwise.
	 */
	AliroError SetBleNotification(BleTypes::AdvertisingServiceData::Notification notification) const;

	/**
	 * @brief Gets the BLE advertising version.
	 *
	 * @return The BLE advertising version.
	 */
	uint8_t GetBleAdvertisingVersion() const;

#endif // CONFIG_ALIRO_BLE_UWB

private:
	Callbacks mCallbacks{};
	AliroConfig mConfig;
	bool mProvisioned{ false };
};

} // namespace Aliro
