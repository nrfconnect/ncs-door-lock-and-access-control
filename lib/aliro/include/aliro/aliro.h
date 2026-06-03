/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

#pragma once

#include "aliro/connection_handle.h"
#include "aliro/errors.h"
#include "aliro/protocol_version.h"
#include "aliro/types.h"

#ifdef CONFIG_NCS_ALIRO_BLE_UWB
#include "aliro/ble_types.h"
#endif // CONFIG_NCS_ALIRO_BLE_UWB

#include <cstddef>

#include <zephyr/sys/util_macro.h>

namespace Aliro {

/**
 * @brief Aliro feature bitmap bit positions.
 *
 */
static constexpr uint8_t kFeatureExpeditedFastPhaseSupported = static_cast<uint8_t>(BIT(0));
static constexpr uint8_t kFeatureStepUpPhaseSupported = static_cast<uint8_t>(BIT(1));
static constexpr uint8_t kFeatureBleUwbSupported = static_cast<uint8_t>(BIT(2));

/**
 * @brief Aliro stack.
 */
class AliroStack {
public:
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
	 * @return ALIRO_NO_ERROR if the stack was initialized successfully, an error code otherwise.
	 */
	AliroError Init();

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

	/**
	 * @brief Gets the enabled features bitmap based on Kconfig options.
	 *
	 * Bit mapping:
	 * - Bit 0: Expedited-fast Phase
	 * - Bit 1: Step-up Phase
	 * - Bit 2: BLE UWB
	 *
	 * @return Bitmap of enabled features.
	 */
	uint8_t GetFeatures() const;

	/**
	 * @brief Create a session for a transport connection.
	 *
	 * Call this when an NFC card is detected and activated or when a BLE connection
	 * is ready to exchange data.
	 *
	 * @param connectionHandle The connection handle identifying the transport connection.
	 *
	 * @return ALIRO_NO_ERROR if the session is created successfully, an error code otherwise.
	 */
	AliroError CreateSession(ConnectionHandle connectionHandle);

	/**
	 * @brief Destroy the session associated with a connection handle.
	 *
	 * Call this when a transport connection is closed or when the application
	 * needs to terminate a session and free resources.
	 *
	 * @param connectionHandle The connection handle identifying the session.
	 */
	void DestroySession(ConnectionHandle connectionHandle);

	/**
	 * @brief Handle data received over a transport connection.
	 *
	 * Call this when data is received from an NFC card or a connected BLE device.
	 *
	 * @param handle The connection handle identifying the transport connection.
	 * @param data The received data.
	 */
	void HandleSessionData(ConnectionHandle handle, Data data);

	/**
	 * @brief Process one deferred stack event.
	 *
	 * Call this from application-owned execution context with an opaque event
	 * pointer previously queued through `Aliro::Interface::Os::QueueEvent()`.
	 *
	 * @param event Opaque pointer to stack event object.
	 */
	void ProcessEvent(void *event);

#ifdef CONFIG_NCS_ALIRO_BLE_UWB

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
				BleTypes::BleExpiryTimestamp expirationTime = BleTypes::kExpiryTimeUnavailable);

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
	AliroError SendReaderStatusChangedMessage(OperationSource operationSource, ReaderStateByte readerState,
						  std::optional<ConnectionHandle> sessionContext = std::nullopt) const;

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
};

} // namespace Aliro
