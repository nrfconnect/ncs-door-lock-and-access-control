/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

#pragma once

#include "aliro/errors.h"
#include "aliro/types.h"

namespace Aliro {

/**
 * @brief Opaque connection handle for platform implementations.
 *
 * This handle represents a connection without exposing internal connection details.
 * Platform implementations should treat this as an opaque pointer.
 */
using ConnectionHandle = void *;

/**
 * @brief Transport error source identifier.
 */
using TransportErrorSource = uint8_t;

/**
 * @brief Transport event callbacks template for platform implementations.
 *
 * This template provides a generic callback structure that can be specialized
 * for different connection handle types. Platform implementations should use
 * the PlatformTransportCallbacks alias.
 *
 * @tparam T The connection identifier type
 */
template <typename T> struct TransportCallbacks {
	/**
	 * @brief Called when data is received and can be further processed.
	 *
	 * @param connection Connection identifier
	 * @param data The received data
	 */
	void (*mOnDataReceived)(T connection, Data data){ nullptr };

	/**
	 * @brief Called when transport is ready and data can be exchanged.
	 *
	 * @param connection Connection identifier
	 */
	void (*mOnTransportReady)(T connection){ nullptr };

	/**
	 * @brief Called on transport error.
	 *
	 * @param connection Connection identifier
	 * @param error The error code
	 * @param source The error source
	 */
	void (*mOnError)(T connection, AliroError error, TransportErrorSource source){ nullptr };

	/**
	 * @brief Called when transport medium is lost.
	 *
	 * @param connection Connection identifier
	 */
	void (*mOnTransportLoss)(T connection){ nullptr };
};

/**
 * @brief Platform transport callbacks using opaque ConnectionHandle.
 *
 * These callbacks allow platform implementations to communicate
 * transport events back to the Aliro stack.
 */
using PlatformTransportCallbacks = TransportCallbacks<ConnectionHandle>;

} // namespace Aliro
