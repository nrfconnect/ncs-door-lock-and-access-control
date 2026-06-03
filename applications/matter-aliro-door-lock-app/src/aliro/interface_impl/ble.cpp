/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifdef CONFIG_NCS_ALIRO_BLE_UWB

#include <aliro_service/aliro_service.h>

/**
 * @file ble_interface_impl.cpp
 * @brief BLE interface helpers for protocol metadata.
 */

namespace Aliro::Interface::Ble {

/**
 * @brief Get the maximum number of concurrent BLE sessions.
 */
size_t GetMaxSessions()
{
	return CONFIG_DOOR_LOCK_BLE_UWB_MAX_SESSIONS;
}

/**
 * @brief Get the protocol version for a connection.
 */
ProtocolVersion GetProtocolVersion(ConnectionHandle handle)
{
	return DoorLock::AliroService::GetProtocolVersion(handle);
}

} // namespace Aliro::Interface::Ble

#endif // CONFIG_NCS_ALIRO_BLE_UWB
