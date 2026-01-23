/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "ble_manager.h"

/**
 * @file ble_interface_impl.cpp
 * @brief This file provides the implementation of the BLE interface functions required by the Aliro stack.
 */

namespace Aliro::Interface::Ble {

/**
 * @brief Send data over an established BLE connection.
 */
AliroError Send(ConnectionHandle handle, Data data)
{
	return BleManager::Instance().Send(handle, data);
}

/**
 * @brief Get the maximum number of concurrent BLE sessions.
 */
size_t GetMaxSessions()
{
	return BleManager::Instance().GetMaxSessions();
}

/**
 * @brief Get the protocol version for a connection.
 */
ProtocolVersion GetProtocolVersion(ConnectionHandle handle)
{
	return BleManager::Instance().GetProtocolVersion(handle);
}

/**
 * @brief Terminate a BLE connection (called by stack to disconnect).
 */
AliroError Terminate(ConnectionHandle handle)
{
	return BleManager::Instance().Terminate(handle);
}

} // namespace Aliro::Interface::Ble
