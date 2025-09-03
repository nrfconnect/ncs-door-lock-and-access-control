/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

#pragma once

#include "aliro/ble_types.h"
#include "aliro/errors.h"

#include <cstdint>

namespace Aliro::BleInterface {

/**
 * @class BleAdvertisingIfc
 * @brief Pure virtual interface for BLE advertising functionality.
 *
 * This interface defines the contract for BLE advertising operations,
 * allowing for dependency inversion and better testability.
 */
class BleAdvertisingIfc {
public:
	virtual ~BleAdvertisingIfc() = default;

	/**
	 * @brief Check if BLE advertising is active.
	 *
	 * @return true if active, false otherwise.
	 */
	virtual bool IsActive() const = 0;

	/**
	 * @brief Start BLE advertising with service configuration.
	 *
	 * @param service Reference to the advertising service configuration.
	 * @param identity The BLE identity to use for advertising.
	 *
	 * @return ALIRO_NO_ERROR when success, error code otherwise.
	 */
	virtual AliroError Start(const BleTypes::AdvertisingService &service, uint8_t identity) = 0;

	/**
	 * @brief Update BLE advertising data.
	 *
	 * @param serviceData Reference to the new advertising service data.
	 * @return ALIRO_NO_ERROR on success, error code otherwise.
	 */
	virtual AliroError UpdateData(const BleTypes::AdvertisingServiceData &serviceData) = 0;

	/**
	 * @brief Stop BLE advertising.
	 *
	 * @return ALIRO_NO_ERROR on success, error code otherwise.
	 */
	virtual AliroError Stop() = 0;

	/**
	 * @brief Get the current transmit power of the BLE radio module.
	 *
	 * @param txPowerLevel Reference to store the TX power level (in dBm).
	 *
	 * @return ALIRO_NO_ERROR on success, error code otherwise.
	 */
	virtual AliroError GetTxPowerLevel(BleTypes::TxPowerLevel &txPowerLevel) const = 0;
};

} // namespace Aliro::BleInterface
