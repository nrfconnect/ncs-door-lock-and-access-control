/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

/**
 * @file ci_cert.h
 * @brief Application hooks for the common Credential Issuer certificate interface implementation.
 *
 * Applications using `DOOR_LOCK_ALIRO_INTERFACE_IMPL_CI_CERT_VALIDATION` must provide
 * implementations of the functions declared in this header.
 */

#pragma once

#include <aliro/interface.h>
#include <aliro/types.h>

namespace DoorLock::InterfaceImpl::CiCert {

/**
 * @brief Result of a Credential Issuer certificate validity period verification.
 */
enum class ValidityPeriodVerificationResult {
	/** Validity period verification is not supported. */
	NotSupported,
	/** Current time is required for verification but is not available. */
	TimeUnavailable,
	/** Current time is outside the certificate validity period. */
	OutsidePeriod,
	/** Current time is within the certificate validity period. */
	WithinPeriod,
};

/**
 * @brief Returns the key ID of the Credential Issuer CA public key.
 *
 * Used by the common Credential Issuer certificate validation to verify
 * certificate signatures.
 *
 * @return The key ID of the Credential Issuer CA public key.
 */
Aliro::CryptoTypes::KeyId GetCredentialIssuerCAPublicKeyId();

/**
 * @brief Verifies the current time against a Credential Issuer certificate validity period.
 *
 * Checks whether the current time falls within the validity period defined by the
 * certificate timestamps.
 *
 * @param timestamps Certificate validity timestamps to check against the current time.
 *
 * @return Verification result.
 */
ValidityPeriodVerificationResult
VerifyCertificateValidityPeriod(const Aliro::Interface::CredentialIssuerCertificate::CertificateTimestamps &timestamps);

} // namespace DoorLock::InterfaceImpl::CiCert
