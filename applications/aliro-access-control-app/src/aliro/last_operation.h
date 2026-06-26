/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
#pragma once

#include <aliro/types.h>

namespace Aliro {

/**
 * @brief Parameters for the last lock or unlock operation reader state change notification.
 */
struct LastOperation {
	OperationSource source{ OperationSource::Unspecified };
	std::optional<CryptoTypes::PublicKey> accessCredentialPublicKey{ std::nullopt };
};

/**
 * @brief Records the last operation parameters for the next reader state change notification.
 *
 * @param isNfcSession `true` when the operation originated from an NFC session, `false` for BLE/UWB.
 * @param accessCredentialPublicKey The Access Credential public key of the User Device that caused the operation.
 */
void SetLastOperation(bool isNfcSession, const CryptoTypes::PublicKey &accessCredentialPublicKey);

/**
 * @brief Records the last operation parameters for the next reader state change notification.
 *
 * @param source The source of the lock or unlock operation.
 */
void SetLastOperation(OperationSource source);

/**
 * @brief Returns the last operation parameters.
 *
 * @return The last operation parameters. Defaults to `OperationSource::Unspecified`.
 */
LastOperation GetLastOperation();

} // namespace Aliro
