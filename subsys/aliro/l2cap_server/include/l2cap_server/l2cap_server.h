/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include <zephyr/bluetooth/l2cap.h>

#include <cstddef>
#include <cstdint>

namespace DoorLock::L2capServer {

/**
 * @struct Callbacks
 * @brief Struct containing callback functions for L2CAP connection events.
 *
 * This struct holds pointers to functions that are called on specific L2CAP
 * connection events.
 */
struct Callbacks {
	/** Called to decide whether an incoming L2CAP connection can be accepted. */
	bool (*mAccept)(bt_conn *){ nullptr };
	/** Called when the L2CAP is connected. */
	void (*mOnConnected)(bt_conn *){ nullptr };
	/** Called when the L2CAP is disconnected. */
	void (*mOnDisconnected)(bt_conn *){ nullptr };
	/** Called when data is received. */
	void (*mOnDataReceived)(bt_conn *, uint8_t *, size_t){ nullptr };
};

/**
 * @brief Initializes and registers the L2CAP server.
 *
 * @param callbacks The callbacks used by the L2CAP server.
 *
 * @return 0 on success, or a negative value on error.
 */
int Init(const Callbacks &callbacks);

/**
 * @brief Get the L2CAP Simplified Protocol/Service Multiplexing (SPSM) value.
 *
 * This function returns the L2CAP SPSM value assigned by the L2CAP server.
 *
 * @return The L2CAP SPSM value.
 */
uint16_t GetSpsm();

/**
 * @brief Sends data over the L2CAP channel.
 *
 * This function sends the provided data over the established L2CAP channel.
 *
 * @param conn The connection to send the data to.
 * @param data Pointer to the data buffer to be sent.
 * @param length Length of the data buffer in bytes.
 *
 * @return 0 on success, or a negative value on error.
 */
int Send(bt_conn *conn, const uint8_t *data, size_t length);

} // namespace DoorLock::L2capServer
