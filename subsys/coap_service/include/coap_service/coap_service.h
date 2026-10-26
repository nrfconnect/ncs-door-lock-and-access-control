/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include "aliro/types.h"

namespace DoorLock::CoAPService {

/**
 * @brief Read the current door lock state for a GET request.
 *
 * @note Called synchronously from the CoAP server thread.
 *
 * @return Current door lock state.
 */
using GetStateCallback = Aliro::ReaderStateByte (*)();

/**
 * @brief Unlock the door lock after PUT request
 *
 * @note Called synchronously from the CoAP server thread.
 * This callback cannot report failure to the client.
 *
 * @param state Requested door lock state.
 */
using UnlockCallback = void (*)();

/**
 * @brief Lock the door lock after PUT request
 *
 * @note Called synchronously from the CoAP server thread.
 * This callback cannot report failure to the client.
 *
 * @param state Requested door lock state.
 */
using LockCallback = void (*)();

/**
 * @brief Initialize the CoAP service callbacks.
 *
 * @note Call before Start(). Do not replace callbacks while the service is running.
 *
 * @param getStateCb Callback for GET requests, or nullptr to reject them with an internal error.
 * @param setStateCb Callback for PUT requests, or nullptr to reject them with an internal error.
 */
void Init(GetStateCallback getStateCb, UnlockCallback unlockCb, LockCallback lockCb);

/**
 * @brief Start the CoAP service over IPv4 UDP.
 *
 * Uses CONFIG_DOOR_LOCK_COAP_SERVICE_PORT (default 5683) and
 * CONFIG_DOOR_LOCK_COAP_RESOURCE_PATH (default "door-lock").
 * Exposes the resource without authentication or encryption.
 *
 * @return 0 on success, or a negative error code on failure.
 */
int Start();

/**
 * @brief Stop the CoAP service.
 *
 * @return 0 on success, or a negative error code on failure.
 */
int Stop();

} // namespace DoorLock::CoAPService
