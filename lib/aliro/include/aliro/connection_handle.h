/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

#pragma once

#include <zephyr/sys/__assert.h>

extern "C" {
/**
 * @brief Forward declaration of bt_conn.
 */
struct bt_conn;
}

namespace Aliro {

/**
 * @brief Unified connection handle for NFC and BLE transport layers.
 *
 * The ConnectionHandle class provides a type-safe way to identify connections
 * across different transport protocols. It can represent either an NFC connection
 * (which doesn't have a persistent connection object) or a BLE connection
 * (which uses a bt_conn pointer).
 *
 * NFC connections are represented by a special sentinel value, while BLE
 * connections store the actual bt_conn pointer.
 */
class ConnectionHandle {
	using HandleType = const void *;

	/** @brief Dummy connection object used as a sentinel for NFC connections. */
	static constexpr int kNfcDummyConnection{};
	/** @brief Connection handle sentinel value for NFC connections. */
	static constexpr HandleType kNfcConnectionHandle{ &kNfcDummyConnection };

public:
	/**
	 * @brief Create a connection handle for an NFC connection.
	 *
	 * @return A ConnectionHandle instance representing an NFC connection.
	 */
	static constexpr ConnectionHandle Nfc() { return ConnectionHandle(kNfcConnectionHandle); }

	/**
	 * @brief Create a connection handle for a BLE connection.
	 *
	 * @param connection Pointer to the BLE connection structure.
	 * @return A ConnectionHandle instance representing the BLE connection.
	 */
	static ConnectionHandle Ble(bt_conn *connection) { return ConnectionHandle(connection); }

	/**
	 * @brief Check if this connection handle represents an NFC connection.
	 *
	 * @return true if this is an NFC connection, false otherwise.
	 */
	constexpr bool IsNfc() const { return mHandle == kNfcConnectionHandle; }

	/**
	 * @brief Check if this connection handle represents a BLE connection.
	 *
	 * @return true if this is a BLE connection, false otherwise.
	 */
	constexpr bool IsBle() const { return mHandle != kNfcConnectionHandle; }

	/**
	 * @brief Get the raw connection handle.
	 *
	 * @return The underlying connection pointer. For NFC connections, this
	 *         returns the sentinel value. For BLE connections, this returns
	 *         the bt_conn pointer.
	 */
	constexpr HandleType GetRaw() const { return mHandle; }

	/**
	 * @brief Get the BLE connection pointer.
	 *
	 * @return Pointer to the bt_conn structure.
	 */
	bt_conn *GetBtConn() const
	{
		if (!IsBle()) {
			__ASSERT(false, "Invalid connection handle");
		}

		return static_cast<bt_conn *>(const_cast<void *>(mHandle));
	}

	/**
	 * @brief Compare two connection handles for equality.
	 *
	 * @param other The connection handle to compare with.
	 * @return true if both connection handles represent the same connection,
	 *         false otherwise.
	 */
	constexpr bool operator==(const ConnectionHandle &other) const { return mHandle == other.mHandle; }

private:
	/**
	 * @brief Private constructor to create a connection handle.
	 *
	 * @param connection The connection pointer or sentinel value.
	 */
	constexpr explicit ConnectionHandle(HandleType connection) : mHandle(connection) {}

	/** @brief The underlying connection handle. */
	HandleType mHandle;
};

} /* namespace Aliro */
