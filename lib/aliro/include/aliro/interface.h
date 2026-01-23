/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

#pragma once

#include "aliro/connection_handle.h"
#include "aliro/errors.h"
#include "aliro/protocol_version.h"
#include "aliro/time.h"
#include "aliro/types.h"

#include <optional>

namespace Aliro::Interface {

namespace ReaderCertificate {

/**
 * @brief Checks if the Reader certificate and Reader System Issuer public key are provisioned.
 *
 * @note Both the Reader certificate and Reader System Issuer public key must be provisioned to use the Reader
 * certificate during the Expedited-standard phase.
 *
 * @return True if the Reader certificate and Reader System Issuer public key are provisioned, false otherwise.
 */
bool IsProvisioned();

/**
 * @brief Gets the Reader System Issuer public key.
 *
 * @param publicKey The Reader System Issuer public key.
 *
 * @return ALIRO_NO_ERROR on success, error code otherwise.
 */
AliroError GetIssuerPublicKey(CryptoTypes::PublicKey &publicKey);

/**
 * @brief Gets the Reader certificate.
 *
 * @param certificate The Reader certificate.
 *
 * @return ALIRO_NO_ERROR on success, error code otherwise.
 */
AliroError GetCertificate(ConstData &certificate);

} // namespace ReaderCertificate

#ifdef CONFIG_NCS_ALIRO_BLE_UWB

namespace Ble {

/**
 * @brief BLE interface functions for Aliro stack integration.
 *
 * This section declares the functions that the application must implement
 * to integrate BLE transport with the Aliro stack. The application is responsible for:
 * - BLE stack initialization
 * - Advertising lifecycle (start/stop/update)
 * - Connection management
 *
 * The application notifies the stack about BLE events by calling the corresponding
 * AliroStack::Indicate* methods.
 *
 * These functions must be implemented in the application scope.
 */

/**
 * @brief Send data over an established BLE connection.
 *
 * @param handle The connection handle identifying the BLE connection.
 * @param data The data to send.
 *
 * @return ALIRO_NO_ERROR on success, error code otherwise.
 */
AliroError Send(ConnectionHandle handle, Data data);

/**
 * @brief Get the maximum number of concurrent BLE sessions supported.
 *
 * @return The maximum number of concurrent BLE sessions.
 */
size_t GetMaxSessions();

/**
 * @brief Get the BLE/UWB protocol version for the given connection.
 *
 * @param handle The connection handle to get the protocol version for.
 *
 * @return The protocol version.
 */
ProtocolVersion GetProtocolVersion(ConnectionHandle handle);

/**
 * @brief Terminate a BLE connection.
 *
 * Called by the stack when a session needs to be terminated
 * (e.g., protocol error, access denied).
 *
 * @param handle The connection handle to terminate.
 *
 * @return ALIRO_NO_ERROR on success, error code otherwise.
 */
AliroError Terminate(ConnectionHandle handle);

} // namespace Ble

namespace Uwb {

/**
 * @brief Handles BLE messages for the UWB module.
 *
 * This function is called by the stack to forward certain BLE messages that require
 * application-level handling. The stack performs decryption and then passes the
 * message with decrypted payload to this function.
 *
 * @note The following message types are forwarded to this function:
 *       - Notification with Ranging Message ID
 *       - UWB Ranging Service
 *
 * @param connectionHandle The connection handle identifying the BLE connection.
 * @param data Pointer to the decrypted message.
 * @param length Length of the message.
 *
 * @return ALIRO_NO_ERROR on success, error code otherwise.
 */
AliroError HandleBleMessage(ConnectionHandle connectionHandle, const uint8_t *data, size_t length);

} // namespace Uwb

#endif // CONFIG_NCS_ALIRO_BLE_UWB

namespace CredentialIssuerCertificate {

/**
 * @brief Structure representing certificate validity timestamps.
 */
struct CertificateTimestamps {
	Time mValidFrom;
	Time mValidUntil;
};

/**
 * @brief Validates an X.509 certificate and extracts the public key and timestamps.
 *
 * @param certificate The DER-encoded certificate to validate.
 * @param publicKey Output parameter for the public key extracted from the certificate.
 * @param timestamps Output parameter for the certificate validity timestamps (valid from/until).
 *
 * @return ALIRO_NO_ERROR on success, error code otherwise.
 */
AliroError Validate(const ConstData &certificate, CryptoTypes::PublicKey &publicKey,
		    std::optional<CertificateTimestamps> &timestamps);

} // namespace CredentialIssuerCertificate

namespace AccessDocument {

/**
 * @brief Verifies the current time against a validity period.
 *
 * Checks whether the current time falls within the specified validity period
 * defined by the validFrom and validUntil timestamps.
 *
 * @param validFrom The start of the validity period (inclusive).
 * @param validUntil The end of the validity period (inclusive).
 *
 * @return True if the current time is within the validity period, false otherwise.
 *         std::nullopt if the verification is not possible.
 */
std::optional<bool> VerifyValidityPeriod(const Time &validFrom, const Time &validUntil);

} // namespace AccessDocument

} // namespace Aliro::Interface
