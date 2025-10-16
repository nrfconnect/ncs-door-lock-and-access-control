/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include "aliro/errors.h"

#include <zephyr/bluetooth/l2cap.h>

#include <cstdint>

namespace Aliro {

/**
 * @class L2capServer
 * @brief Implements the L2CAP server for the Aliro service.
 */
class L2capServer {
public:
	using Spsm = uint16_t;

	static_assert(sizeof(L2capServer::Spsm) == sizeof(uint16_t),
		      "Spsm type must be 2 bytes, as implementation depends on it");

	/**
	 * @struct Callbacks
	 * @brief Struct containing callback functions for L2CAP connection events.
	 *
	 * This struct holds pointers to functions that are called on specific L2CAP
	 * connection events.
	 */
	struct Callbacks {
		/** Called when the L2CAP is connected. */
		void (*mOnConnected)(bt_conn *){ nullptr };
		/** Called when the L2CAP is disconnected. */
		void (*mOnDisconnected)(bt_conn *){ nullptr };
		/** Called when data is received. */
		void (*mOnDataReceived)(bt_conn *, uint8_t *, size_t){ nullptr };
	};

	/**
	 * @brief Returns instance of the L2CAP server.
	 *
	 * @return Reference to the L2CAP server instance.
	 */
	static L2capServer &Instance()
	{
		static L2capServer sInstance;
		return sInstance;
	}

	// No copy, no move and no assigment allowed.
	L2capServer(const L2capServer &) = delete;
	L2capServer &operator=(const L2capServer &) = delete;
	L2capServer(L2capServer &&) = delete;
	L2capServer &operator=(L2capServer &&) = delete;

	/**
	 * @brief Initializes and registers the L2CAP server.
	 * The method must be called before using any other methods of this class.
	 *
	 * @return ALIRO_NO_ERROR on success, or an error code on failure.
	 */
	AliroError Init();

	/**
	 * @brief Get the L2CAP Simplified Protocol/Service Multiplexing (SPSM) value.
	 *
	 * This function returns the L2CAP SPSM value assigned by the L2CAP server.
	 *
	 * @return The L2CAP SPSM value.
	 */
	Spsm GetSpsm() const;

	/**
	 * @brief Checks if the given SPSM value is valid for dynamic allocation.
	 *
	 * This function checks if the provided SPSM value falls within the
	 * range of dynamically allocated values (0x0080 to 0x00FF).
	 *
	 * @param spsm The SPSM value to check.
	 *
	 * @return true if the SPSM value is valid, false otherwise.
	 */
	static bool IsValidDynamicSpsm(Spsm spsm);

	/**
	 * @brief Sends data over the L2CAP channel.
	 *
	 * This function sends the provided data over the established L2CAP channel.
	 *
	 * @param conn The connection to send the data to.
	 * @param data Pointer to the data buffer to be sent.
	 * @param length Length of the data buffer in bytes.
	 *
	 * @return ALIRO_NO_ERROR on success, or an error code on failure:
	 *         - ALIRO_INVALID_STATE if channel is not connected
	 *         - ALIRO_INVALID_ARGUMENT if data is invalid
	 *         - ALIRO_NO_MEMORY if no buffers available
	 */
	AliroError Send(bt_conn *conn, const uint8_t *data, size_t length) const;

	/**
	 * @brief Sets the callbacks for L2CAP events.
	 *
	 * This function sets the callbacks that will be called when L2CAP events occur,
	 * such as connection, disconnection, and data reception.
	 *
	 * @param callbacks The callbacks structure containing the event handlers.
	 */
	void SetCallbacks(Callbacks callbacks) { mCallbacks = callbacks; }

private:
	/**
	 * @brief Default constructor for L2capServer.
	 */
	L2capServer() = default;

	/**
	 * @brief Accepts an incoming L2CAP connection.
	 *
	 * This function is called when a new incoming connection requires
	 * authorization. It allocates the channel structure to be used by the
	 * new connection.
	 *
	 * @param connectionId The connection that is requesting authorization.
	 * @param server Pointer to the server structure this callback relates to.
	 * @param channel Pointer to receive the allocated channel.
	 *
	 * @return 0 on success, or a negative value on error.
	 */
	static int Accept(bt_conn *connectionId, bt_l2cap_server *server, bt_l2cap_chan **channel);

	/**
	 * @brief Channel connected callbacks.
	 */
	static void Connected(bt_l2cap_chan *channel);
	static void Disconnected(bt_l2cap_chan *channel);
	static int DataReceived(bt_l2cap_chan *channel, net_buf *buffer);
	static void Released(bt_l2cap_chan *channel);

	Spsm mSpsm{};

	Callbacks mCallbacks{};

	bt_l2cap_server mL2capServer{};

	bt_l2cap_chan_ops mChannelCallbacks{};

	size_t mChannelCount{ 0 };

	// Use a non-SIG assigned SPSM value for the L2CAP server.
	constexpr static uint16_t kL2capSpsmMin = 0x0080;
	constexpr static uint16_t kL2capSpsmMax = 0x00FF;
};

} // namespace Aliro
