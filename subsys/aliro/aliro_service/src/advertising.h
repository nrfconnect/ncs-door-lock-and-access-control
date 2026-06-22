/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include <aliro/ble_types.h>

namespace DoorLock::AliroService::Advertising {

/**
 * Initializes the Aliro advertising set.
 *
 * @param address BLE address of the identity used for Aliro advertising.
 * @return 0 on success, otherwise a negative errno-style error code.
 */
int Init(const Aliro::BleTypes::BleAddress &address);

/**
 * Generates and applies Aliro advertising and scan response data.
 *
 * The advertising payload is generated from the stored Reader identifier,
 * current TX power, configured BLE address and provided parameters.
 *
 * @param notification Notification value to include in the Aliro service data.
 * @param expiryTime Expiry timestamp to include in the Aliro service data.
 * @return 0 on success, otherwise a negative errno-style error code.
 */
int SetData(Aliro::BleTypes::AdvertisingServiceData::Notification notification,
	    Aliro::BleTypes::BleExpiryTimestamp expiryTime);

/**
 * Starts Aliro BLE advertising.
 *
 * @return 0 on success, otherwise a negative errno-style error code.
 */
int Start();

/**
 * Stops Aliro BLE advertising.
 *
 * @return 0 on success, otherwise a negative errno-style error code.
 */
int Stop();

} // namespace DoorLock::AliroService::Advertising
