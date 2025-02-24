/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include "errors.h"
#include "transport_protocol/transport_protocol_nfc.h"

/**
 * @brief Called when a user device is detected by the NFC reader.
 *
 * Initiates the NFC communication by sending a SELECT command.
 *
 * @return AliroError Result of the operation.
 */
AliroError AliroUserDeviceDetectedClb();

/**
 * @brief Handles the reception of a SELECT command.
 *
 * Processes the raw data received from the NFC interface when a SELECT command is received.
 *
 * @param rawData Pointer to the raw data received.
 * @param dataLen Length of the data received.
 * @return AliroError Result of the operation.
 */
AliroError AliroSelectCommandRecvClb(uint8_t *rawData, size_t dataLen);

/**
 * @brief Error handler for NFC communication errors.
 *
 * This function is called when an error occurs during NFC communication. It sends control flow command
 * or terminates the secure session based on the error status.
 *
 * @param statusCode The error status code indicating the type of error encountered.
 * @return AliroError Result of the operation.
 */
AliroError AliroErrorHandlerClb(Aliro::AliroNfcErrorStatusCode statusCode);

/**
 * @brief Processes the response to an AUTH0 command.
 *
 * This function handles the reception of an AUTH0 response, prepares and sends an AUTH1 command, and establishes a
 * secure channel based on the received credentials.
 *
 * @param rawData Pointer to the raw data received.
 * @param dataLen Length of the data received.
 * @return AliroError Result of the operation.
 */
AliroError AliroAuth0RecvClb(uint8_t *rawData, size_t dataLen);

/**
 * @brief Processes the response to an AUTH1 command.
 *
 * Handles the reception of an AUTH1 response, verifies the signature of the user device, and determines access control
 * status based on the verification result.
 *
 * @param rawData Pointer to the raw data received.
 * @param dataLen Length of the data received.
 * @return AliroError Result of the operation.
 */
AliroError AliroAuth1RecvClb(uint8_t *rawData, size_t dataLen);

/**
 * @brief Ends the NFC communication session.
 *
 * Performs necessary cleanup, such as closing secure channels, and ends the NFC communication session.
 *
 * @return AliroError Result of the operation.
 */
AliroError AliroCommunicationEndClb();
