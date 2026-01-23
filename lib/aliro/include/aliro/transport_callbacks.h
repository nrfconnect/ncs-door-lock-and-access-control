/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

#pragma once

#include "aliro/connection_handle.h"
#include "aliro/errors.h"
#include "aliro/types.h"

namespace Aliro {

/**
 * @brief Transport error source identifier.
 */
using TransportErrorSource = uint8_t;

/**
 * @brief Transport event callbacks template for internal stack use.
 *
 * This template provides a generic callback structure for NFC transport events.
 * *
 * @tparam T The connection handle type
 */
template <typename T> struct TransportCallbacks {
	/**
	 * @brief Called when data is received and can be further processed.
	 *
	 * @param connection Connection handle
	 * @param data The received data
	 */
	void (*mOnDataReceived)(T connection, Data data){ nullptr };

	/**
	 * @brief Called when transport is ready and data can be exchanged.
	 *
	 * @param connection Connection handle
	 */
	void (*mOnTransportReady)(T connection){ nullptr };

	/**
	 * @brief Called on transport error.
	 *
	 * @param connection Connection handle
	 * @param error The error code
	 * @param source The error source
	 */
	void (*mOnError)(T connection, AliroError error, TransportErrorSource source){ nullptr };

	/**
	 * @brief Called when transport medium is lost.
	 *
	 * @param connection Connection handle
	 */
	void (*mOnTransportLoss)(T connection){ nullptr };
};

/**
 * @brief Platform transport callbacks using ConnectionHandle.
 *
 * Note: These callbacks are only used internally by the NFC transport layer.
 * For BLE transport, the application uses AliroStack::Indicate* methods directly.
 */
using PlatformTransportCallbacks = TransportCallbacks<ConnectionHandle>;

} // namespace Aliro
