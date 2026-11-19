/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include "aliro/errors.h"
#include "aliro/types.h"

/**
 * @brief NFC transport used by the Aliro stack.
 *
 * Connects the NFC transport backend to the Aliro stack: reader events reported by
 * the backend are forwarded to the Aliro stack as NFC session events.
 */
namespace Aliro::NfcTransport {

/**
 * @brief Initializes the NFC transport.
 * @return ALIRO_NO_ERROR on success, error code otherwise.
 */
AliroError Init();

/**
 * @brief Starts NFC polling/discovery.
 * @return ALIRO_NO_ERROR on success, error code otherwise.
 */
AliroError Start();

/**
 * @brief Stops NFC polling.
 * @return ALIRO_NO_ERROR on success, error code otherwise.
 */
AliroError Stop();

/**
 * @brief Sends data to the detected NFC device.
 *
 * @param data The data to send.
 * @return ALIRO_NO_ERROR on success, error code otherwise.
 */
AliroError Send(Data data);

/**
 * @brief Terminates the current NFC session and restarts polling.
 *
 * @return ALIRO_NO_ERROR on success, error code otherwise.
 */
AliroError Terminate();

} // namespace Aliro::NfcTransport
