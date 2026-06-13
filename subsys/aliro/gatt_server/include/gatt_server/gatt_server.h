/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include <aliro/protocol_version.h>

#include <zephyr/bluetooth/conn.h>

#include <cstdint>

namespace DoorLock::GattServer {

/** Supported features bitmap. */
struct SupportedFeatures {
	uint8_t TimesyncProcedure0 : 1;
	uint8_t TimesyncProcedure1 : 1;
	uint8_t LeCodedPhy : 1;
	uint8_t : 5;
} __packed;

/**
 * @brief Initializes the GATT server.
 *
 * @param spsm L2CAP SPSM value for the Aliro channel.
 * @param supportedFeatures Supported features bitmap.
 *
 * @return 0 on success, a negative errno value otherwise.
 */
int Init(uint16_t spsm, const SupportedFeatures &supportedFeatures);

/**
 * @brief Registers the GATT server.
 *
 * @return 0 on success, a negative errno value otherwise.
 */
int Register();

/**
 * @brief Unregisters the GATT server.
 *
 * @return 0 on success, a negative errno value otherwise.
 */
int Unregister();

/**
 * @brief Gets the BLE UWB protocol version for the given connection.
 */
Aliro::ProtocolVersion GetBleUwbProtocolVersion(const bt_conn *conn);

} // namespace DoorLock::GattServer
