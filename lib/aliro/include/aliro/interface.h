/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

#pragma once

#include "aliro/errors.h"
#include "aliro/types.h"

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

} // namespace Aliro::Interface
