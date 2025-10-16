/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

#pragma once

#include "aliro/ble_types.h"
#include "aliro/errors.h"
#include "aliro/transport_callbacks.h"
#include "aliro/types.h"

#include <cstdint>

namespace Aliro::BleInterface {

class BleIfc {
public:
	virtual ~BleIfc() = default;

	/**
	 * @brief Initialize the BLE connection.
	 *
	 * @param callbacks Transport callbacks for event notification.
	 *                  Platform implementations use these to notify the stack
	 *                  of transport events.
	 *
	 * @return ALIRO_NO_ERROR on success, error code otherwise.
	 */
	virtual AliroError Init(const PlatformTransportCallbacks &callbacks) = 0;

	/**
	 * @brief Send data over the BLE connection.
	 *
	 * @param id The connection ID.
	 * @param data The data to send.
	 *
	 * @return ALIRO_NO_ERROR on success, error code otherwise.
	 */
	virtual AliroError Send(ConnectionHandle handle, Data data) const = 0;

	/**
	 * @brief Disconnect the BLE connection.
	 *
	 * @return ALIRO_NO_ERROR on success, error code otherwise.
	 */
	virtual AliroError Disconnect(ConnectionHandle handle) = 0;

	/**
	 * @brief Disconnect all BLE connections.
	 */
	virtual void DisconnectAll() = 0;

	/**
	 * @brief Start BLE advertising with service configuration.
	 *
	 * @param data Reference to the advertising service data. For service data types (Uuid16, Uuid32, Uuid128),
	 * this contains the service UUID followed by additional payload data. For complete service list types
	 * (Uuid16All, Uuid32All, Uuid128All), this contains only the service UUIDs without additional data.
	 * @param type The type of advertising data field.
	 *
	 * @return ALIRO_NO_ERROR when success, error code otherwise.
	 */
	virtual AliroError StartAdvertising(const ConstData &data, BleTypes::AdvertisingDataFieldType type) = 0;

	/**
	 * @brief Update BLE advertising data.
	 *
	 * @param data Reference to the advertising service data. For service data types (Uuid16, Uuid32, Uuid128),
	 * this contains the service UUID followed by additional payload data. For complete service list types
	 * (Uuid16All, Uuid32All, Uuid128All), this contains only the service UUIDs without additional data.
	 * @param type The type of advertising data field.
	 *
	 * @return ALIRO_NO_ERROR on success, error code otherwise.
	 */
	virtual AliroError UpdateAdvertisingData(const ConstData &data, BleTypes::AdvertisingDataFieldType type) = 0;

	/**
	 * @brief Stop BLE advertising.
	 *
	 * @return ALIRO_NO_ERROR on success, error code otherwise.
	 */
	virtual AliroError StopAdvertising() = 0;

	/**
	 * @brief Get the BLE address.
	 *
	 * @param address Reference to store the BLE address.
	 *
	 * @return ALIRO_NO_ERROR on success, error code otherwise.
	 */
	virtual AliroError GetAddress(BleTypes::BleAddress &address) const = 0;

	/**
	 * @brief Get the current transmit power of the BLE radio module.
	 *
	 * @param txPowerLevel Reference to store the TX power level (in dBm).
	 *
	 * @return ALIRO_NO_ERROR on success, error code otherwise.
	 */
	virtual AliroError GetTxPowerLevel(BleTypes::TxPowerLevel &txPowerLevel) const = 0;

	/**
	 * @brief Get the maximum number of BLE sessions.
	 *
	 * @return The maximum number of concurrent BLE sessions supported.
	 */
	virtual size_t GetMaxSessions() const = 0;
};

} // namespace Aliro::BleInterface
