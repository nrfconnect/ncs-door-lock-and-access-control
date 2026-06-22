/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include <aliro/connection_handle.h>
#include <aliro/protocol_version.h>
#include <aliro/types.h>

namespace DoorLock::AliroService {

/**
 * Reasons an application component can use to temporarily suppress Aliro
 * advertising.
 *
 * Advertising remains blocked while at least one reason is active. Each reason
 * must be cleared with UnblockAdvertising() before the service may advertise
 * again.
 */
enum class AdvertisingBlockReason {
	/**
	 * Advertising is blocked while the door is already unlocked.
	 */
	DoorUnlocked,
};

/**
 * Initializes the Aliro service.
 *
 * @return 0 on success, or a negative error code on failure.
 */
int Init();

/**
 * Starts the Aliro service.
 *
 * Registers the Aliro GATT service and starts advertising if all block reasons are cleared.
 *
 * @return 0 on success, or a negative error code on failure.
 */
int Start();

/**
 * Stops advertising, disconnects active Aliro BLE connections, and unregisters
 * the Aliro GATT service.
 *
 * @return 0 on success, or a negative error code on failure.
 */
int Stop();

/**
 * Prevents Aliro advertising for the specified reason.
 *
 * If the service is started and this call adds the first active block reason,
 * current advertising is stopped. Calling this repeatedly for the same reason
 * has no effect.
 *
 * @param reason Reason advertising should be blocked.
 * @return 0 on success, or a negative error code on failure.
 */
int BlockAdvertising(AdvertisingBlockReason reason);

/**
 * Removes a previously registered advertising block reason.
 *
 * Advertising is started only after the last active block reason has been cleared,
 * the service is started, and the configured session limit has not been reached.
 * Calling this for a reason that is not active has no effect.
 *
 * @param reason Reason advertising should no longer be blocked for.
 * @return 0 on success, or a negative error code on failure.
 */
int UnblockAdvertising(AdvertisingBlockReason reason);

/**
 * Refreshes Aliro advertising data.
 *
 * Schedules regeneration of the advertising payload. Should be called when data that
 * is encoded in Aliro advertising has changed, for example:
 * - Reader Group or Sub-Group Identifier changes.
 * - Transmit power changes.
 * - Local time is synchronized with the time server.
 *
 * If the service is not initialized, the request is ignored.
 */
void RefreshAdvertising();

/**
 * Sends Aliro data over an active connection.
 *
 * @param handle Connection handle identifying the target peer.
 * @param data Data payload to send.
 * @return 0 on success, or a negative error code on failure.
 */
int Send(Aliro::ConnectionHandle handle, Aliro::Data data);

/**
 * Terminates an active Aliro connection.
 *
 * @note This function must be called by the application after a BLE session has been destroyed.
 *
 * @param handle Connection handle identifying the peer to disconnect.
 * @return 0 on success, or a negative error code on failure.
 */
int Terminate(Aliro::ConnectionHandle handle);

/**
 * Gets the negotiated Aliro protocol version associated with a connection.
 *
 * @param handle Connection handle identifying the peer.
 * @return Protocol version used for the connection.
 */
Aliro::ProtocolVersion GetProtocolVersion(Aliro::ConnectionHandle handle);

} // namespace DoorLock::AliroService
