/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include <zephyr/bluetooth/uuid.h>

/** Aliro service UUID value. */
#define BT_UUID_ALIRO_SERVICE_VAL 0xFFF2

/** Aliro service UUID. */
#define BT_UUID_ALIRO_SERVICE BT_UUID_DECLARE_16(BT_UUID_ALIRO_SERVICE_VAL)

/** Reader SPSM and Aliro BLE UWB Protocol Version characteristic UUID value. */
#define BT_UUID_ALIRO_READER_SPSM_BLE_UWB_PROTOCOL_VERSION_CHARACTERISTIC_VAL                                          \
	BT_UUID_128_ENCODE(0xD3B5A130, 0x9E23, 0x4B3A, 0x8BE4, 0x6B1EE5F980A3)

/** Reader SPSM and Aliro BLE UWB Protocol Version characteristic UUID. */
#define BT_UUID_ALIRO_READER_SPSM_BLE_UWB_PROTOCOL_VERSION_CHARACTERISTIC                                              \
	BT_UUID_DECLARE_128(BT_UUID_ALIRO_READER_SPSM_BLE_UWB_PROTOCOL_VERSION_CHARACTERISTIC_VAL)

/** User Device Selected Aliro BLE UWB Protocol Version characteristic UUID value. */
#define BT_UUID_ALIRO_DEVICE_BLE_UWB_PROTOCOL_VERSION_CHARACTERISTIC_VAL                                               \
	BT_UUID_128_ENCODE(0xBD4B9502, 0x3F54, 0x11EC, 0xB919, 0x0242AC120005)

/** User Device Selected Aliro BLE UWB Protocol Version characteristic UUID. */
#define BT_UUID_ALIRO_DEVICE_BLE_UWB_PROTOCOL_VERSION_CHARACTERISTIC                                                   \
	BT_UUID_DECLARE_128(BT_UUID_ALIRO_DEVICE_BLE_UWB_PROTOCOL_VERSION_CHARACTERISTIC_VAL)
