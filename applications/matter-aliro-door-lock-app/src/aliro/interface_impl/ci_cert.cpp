/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifdef CONFIG_DOOR_LOCK_ALIRO_INTERFACE_IMPL_CI_CERT_VALIDATION

#include <interface_impl/ci_cert.h>

#include "../storage/psa_key_ids.h"

#ifdef CONFIG_DOOR_LOCK_TIME_CONCEPT
#include <time_utils/time_utils.h>

#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(interface_ci_cert, CONFIG_DOOR_LOCK_APP_LOG_LEVEL);
#endif // CONFIG_DOOR_LOCK_TIME_CONCEPT

Aliro::CryptoTypes::KeyId DoorLock::InterfaceImpl::CiCert::GetCredentialIssuerCAPublicKeyId()
{
	return DoorLock::Storage::PsaKeyIds::kCredentialIssuerCAPublicKeyId;
}

DoorLock::InterfaceImpl::CiCert::ValidityPeriodVerificationResult
DoorLock::InterfaceImpl::CiCert::VerifyCertificateValidityPeriod(
	const Aliro::Interface::CredentialIssuerCertificate::CertificateTimestamps &timestamps)
{
#ifdef CONFIG_DOOR_LOCK_TIME_CONCEPT
	const auto currentTimeOpt = DoorLock::TimeUtils::GetCurrentTime();
	if (!currentTimeOpt.has_value()) {
		return ValidityPeriodVerificationResult::TimeUnavailable;
	}

	const auto &currentTime = currentTimeOpt.value();
	const auto &validFrom = timestamps.mValidFrom;
	const auto &validUntil = timestamps.mValidUntil;

	LOG_DBG("Current time: %04d-%02d-%02d %02d:%02d:%02d", currentTime.mYear, currentTime.mMonth, currentTime.mDay,
		currentTime.mHour, currentTime.mMinute, currentTime.mSecond);
	LOG_DBG("validFrom   : %04d-%02d-%02d %02d:%02d:%02d", validFrom.mYear, validFrom.mMonth, validFrom.mDay,
		validFrom.mHour, validFrom.mMinute, validFrom.mSecond);
	LOG_DBG("validUntil  : %04d-%02d-%02d %02d:%02d:%02d", validUntil.mYear, validUntil.mMonth, validUntil.mDay,
		validUntil.mHour, validUntil.mMinute, validUntil.mSecond);

	if (currentTime < validFrom || validUntil < currentTime) {
		return ValidityPeriodVerificationResult::OutsidePeriod;
	}

	return ValidityPeriodVerificationResult::WithinPeriod;
#else
	return ValidityPeriodVerificationResult::NotSupported;
#endif // CONFIG_DOOR_LOCK_TIME_CONCEPT
}

#endif // CONFIG_DOOR_LOCK_ALIRO_INTERFACE_IMPL_CI_CERT_VALIDATION
