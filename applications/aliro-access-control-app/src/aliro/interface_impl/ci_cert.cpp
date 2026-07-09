/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifdef CONFIG_DOOR_LOCK_ALIRO_INTERFACE_IMPL_CI_CERT_VALIDATION

#include <interface_impl/ci_cert.h>

#include "../storage/psa_key_ids.h"

Aliro::CryptoTypes::KeyId DoorLock::InterfaceImpl::CiCert::GetCredentialIssuerCAPublicKeyId()
{
	return DoorLock::Storage::PsaKeyIds::kCredentialIssuerCAPublicKeyId;
}

DoorLock::InterfaceImpl::CiCert::ValidityPeriodVerificationResult
DoorLock::InterfaceImpl::CiCert::VerifyCertificateValidityPeriod(
	const Aliro::Interface::CredentialIssuerCertificate::CertificateTimestamps &)
{
	return ValidityPeriodVerificationResult::NotSupported;
}

#endif // CONFIG_DOOR_LOCK_ALIRO_INTERFACE_IMPL_CI_CERT_VALIDATION
