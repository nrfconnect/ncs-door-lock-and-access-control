/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include <cstddef>
#include <cstdint>

namespace DoorLock::NUSService {

using CommandCallback = void (*)(void *context);

/**
 * @brief Initialize the NUS service.
 *
 * @return 0 on success, or a negative error code on failure.
 */
int Init();

/**
 * @brief Start the NUS service.
 *
 * @param priority Advertising priority.
 * @param minInterval Advertising interval minimum.
 * @param maxInterval Advertising interval maximum.
 * @return 0 on success, or a negative error code on failure.
 */
int Start(uint8_t priority, uint16_t minInterval, uint16_t maxInterval);

/**
 * @brief Stop the Nordic UART Service server.
 */
void Stop();

/**
 * @brief Send data to the connected device.
 *
 * @param data Buffer to send.
 * @param length Number of bytes to send.
 * @return 0 on success, or a negative error code on failure.
 */
int Send(const char *const data, size_t length);

/**
 * @brief Send data to the connected device.
 *
 * @param data Buffer to send.
 * @return 0 on success, or a negative error code on failure.
 */
template <size_t N> int Send(const char (&data)[N])
{
	return Send(data, N - 1);
}

/**
 * @brief Register a new command for NUS service.
 *
 * @param name Command token bytes; `length` is the number of bytes compared at the start of a line.
 * @param length Length of the command name in bytes.
 * @param callback Callback invoked when the command matches (may be null).
 * @param context User context passed to the callback.
 * @return 0 on success, or a negative error code on failure.
 */
int RegisterCommand(const char *const name, size_t length, CommandCallback callback, void *context);

/**
 * @brief Register a new command for NUS service.
 *
 * @note Length of the command name is determined by the compile-time size of the array.
 *
 * @param name Command token bytes;
 * @param callback Callback invoked when the command matches (may be null).
 * @param context User context passed to the callback.
 * @return 0 on success, or a negative error code on failure.
 */
template <size_t N> int RegisterCommand(const char (&name)[N], CommandCallback callback, void *context)
{
	return RegisterCommand(name, N - 1, callback, context);
}

} // namespace DoorLock::NUSService
