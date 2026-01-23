/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

#pragma once

#include "access_manager/access_manager.h"
#include "aliro/errors.h"
#include "aliro/protocol_version.h"
#include "aliro/transport_callbacks.h"
#include "aliro/types.h"
#include "kpersistent_manager/kpersistent_manager.h"

#ifdef CONFIG_NCS_ALIRO_BLE_UWB
#include "aliro/ble_types.h"
#endif // CONFIG_NCS_ALIRO_BLE_UWB

#include <cstddef>

namespace Aliro {

struct AliroConfig {
	/**
	 * @brief Enable NFC transport.
	 */
	bool mEnableNfc{ true };

	/**
	 * @brief The Kpersistent manager needed for the Expedited-fast phase.
	 */
	[[maybe_unused]] KpersistentManager *mKpersistentManager{};
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
	 * @note After updating the identifier, the application must refresh the BLE advertising with the data generated
	 *
	 * @param identifier The reader identifier.
	 *
	 * @return ALIRO_NO_ERROR if the reader is provisioned, an error code otherwise.
	 */
	AliroError SetReaderIdentifier(const Identifier &identifier);

	/**
	 * @brief Gets the reader identifier.
	 *
	 * @param identifier The reader identifier.
	 *
	 * @return ALIRO_NO_ERROR if the reader identifier was retrieved successfully, an error code otherwise.
	 */
	AliroError GetReaderIdentifier(Identifier &identifier) const;

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

	/**
	 * @brief Gets the Expedited Standard protocol version.
	 *
	 * @param versionCount The number of protocol versions.
	 *
	 * @return Pointer to the static Expedited Standard protocol version list.
	 */
	const ProtocolVersion *GetExpeditedStandardProtocolVersions(size_t &versionCount) const;

#ifdef CONFIG_NCS_ALIRO_BLE_UWB

	/* These methods are called by the application to notify the stack about BLE events */

	/**
	 * @brief Indicate that a BLE transport is ready for communication.
	 *
	 * Call this when a BLE connection is established and the GATT service
	 * is ready to exchange data (e.g., after CCCD subscription).
	 *
	 * @param handle The connection handle identifying the BLE connection.
	 */
	void IndicateTransportReady(ConnectionHandle handle);

	/**
	 * @brief Indicate that a BLE transport connection was lost.
	 *
	 * Call this when a BLE connection is disconnected.
	 *
	 * @param handle The connection handle identifying the BLE connection.
	 */
	void IndicateTransportLoss(ConnectionHandle handle);

	/**
	 * @brief Indicate that data was received over BLE transport.
	 *
	 * Call this when data is received from a connected BLE device.
	 *
	 * @param handle The connection handle identifying the BLE connection.
	 * @param data The received data.
	 */
	void IndicateDataReceived(ConnectionHandle handle, Data data);

	/**
	 * @brief Generate Aliro BLE advertising service data.
	 *
	 * Call this to get the advertising payload when starting or refreshing
	 * BLE advertising. The application provides BLE-specific parameters
	 * (address, TX power, notification, expiration time) that it controls.
	 *
	 * @param outData The output advertising service data.
	 * @param address The current BLE advertising address.
	 * @param txPowerLevel The current TX power level in dBm.
	 * @param readerIdentifier The reader identifier (32 bytes: reader_group_id | reader_group_sub_id).
	 * @param notification The notification type.
	 * @param expirationTime The expiration timestamp.
	 *
	 * @return ALIRO_NO_ERROR on success, error code otherwise.
	 */
	static AliroError
	GenerateAdvertisingData(BleTypes::AdvertisingServiceData &outData, const BleTypes::BleAddress &address,
				BleTypes::TxPowerLevel txPowerLevel, const Identifier &readerIdentifier,
				BleTypes::AdvertisingServiceData::Notification notification =
					BleTypes::AdvertisingServiceData::Notification::NoError,
				const BleTypes::BleExpiryTimestamp &expirationTime = BleTypes::kExpiryTimeUnavailable);

	/**
	 * @brief Gets the BLE advertising version.
	 *
	 * @return The BLE advertising version.
	 */
	static uint8_t GetBleAdvertisingVersion();

	/**
	 * @brief Sends the Reader Status Changed Message.
	 *
	 * @param operationSource The operation source that caused the state change.
	 * @param readerState The current reader state.
	 * @param sessionContext Optionally the session context for specific User Device.
	 *
	 * @return ALIRO_NO_ERROR if successful, an error code otherwise.
	 */
	AliroError SendReaderStatusChangedMessage(
		OperationSource operationSource, ReaderStateByte readerState,
		std::optional<AccessManager::SessionContext> sessionContext = std::nullopt) const;

	/**
	 * @brief Gets the BLE/UWB protocol version.
	 *
	 * @param versionCount The number of protocol versions.
	 *
	 * @return Pointer to the static BLE/UWB protocol version list.
	 */
	const ProtocolVersion *GetBleUwbProtocolVersions(size_t &versionCount) const;

	/**
	 * @brief Sends a BLE message from the UWB module to the stack.
	 *
	 * This function is called by the application to send messages that require
	 * stack-level encryption and transmission. The stack encrypts the message payload and
	 * sends it over the BLE connection.
	 *
	 * @note The following message types may be sent through this function:
	 *       - Notification with Ranging Message ID
	 *       - UWB Ranging Service
	 *
	 * @param connectionHandle The connection handle identifying the BLE connection.
	 * @param data Pointer to the unencrypted message.
	 * @param length Length of the message in bytes.
	 */
	void SendBleMessage(ConnectionHandle connectionHandle, const uint8_t *data, size_t length) const;

#endif // CONFIG_NCS_ALIRO_BLE_UWB

private:
	Callbacks mCallbacks{};
	AliroConfig mConfig{};
	bool mProvisioned{ false };
};

} // namespace Aliro
