/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "credential_issuer_keys.h"

#include "psa_ps_ids.h"

#include <access_manager.h>
#include <doorlock/utils/utils.h>

#include <psa/protected_storage.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(credential_issuer_keys, CONFIG_DOOR_LOCK_APP_LOG_LEVEL);

namespace Aliro {

namespace {

using namespace DoorLock::Storage::PsaPsIds;

constexpr size_t kMaxStoredKeys{ CONFIG_DOOR_LOCK_ACCESS_MANAGER_CREDENTIAL_ISSUER_CERTIFICATE_MAX_STORED_KEYS };

struct StoredCredentialIssuer {
	static constexpr uint8_t kVersion{ 1 };

	uint8_t mVersion{ kVersion };
	CryptoTypes::PublicKey mPublicKey{};
	ValidityPeriod mValidityPeriod{};
};

static_assert(sizeof(StoredCredentialIssuer) == sizeof(StoredCredentialIssuer::kVersion) +
							     CryptoTypes::kEccP256PublicKeyLength +
							     sizeof(ValidityPeriod));

psa_storage_uid_t GetUid(size_t index)
{
	return kCertificateCredentialIssuerUidBegin + index;
}

} // namespace

AliroError StoreCertificateCredentialIssuerKey(size_t index, const CryptoTypes::PublicKey &publicKey,
					       const ValidityPeriod &validityPeriod)
{
	VerifyOrReturnValue(index < kMaxStoredKeys, ALIRO_INVALID_ARGUMENT,
			    LOG_ERR("Certificate Credential Issuer key index out of range"));

	const StoredCredentialIssuer record{ .mPublicKey = publicKey, .mValidityPeriod = validityPeriod };
	const psa_status_t status =
		psa_ps_set(GetUid(index), sizeof(record), &record, PSA_STORAGE_FLAG_NONE);
	VerifyOrReturnValue(status == PSA_SUCCESS, ALIRO_ERROR_INTERNAL,
			    LOG_ERR("Failed to store Certificate Credential Issuer key (psa=%d)", status));

	return ALIRO_NO_ERROR;
}

AliroError ReadCertificateCredentialIssuerKey(size_t index, CryptoTypes::PublicKey &publicKey,
					      ValidityPeriod *validityPeriod)
{
	VerifyOrReturnValue(index < kMaxStoredKeys, ALIRO_INVALID_ARGUMENT,
			    LOG_ERR("Certificate Credential Issuer key index out of range"));

	psa_storage_info_t storageInfo{};
	psa_status_t status = psa_ps_get_info(GetUid(index), &storageInfo);
	if (status == PSA_ERROR_DOES_NOT_EXIST) {
		return ALIRO_PUBLIC_KEY_NOT_FOUND;
	}
	VerifyOrReturnValue(status == PSA_SUCCESS, ALIRO_ERROR_INTERNAL,
			    LOG_ERR("Failed to get Certificate Credential Issuer key info (psa=%d)", status));
	VerifyOrReturnValue(storageInfo.size == sizeof(StoredCredentialIssuer), ALIRO_ERROR_INTERNAL,
			    LOG_ERR("Invalid Certificate Credential Issuer record length: %u",
				    static_cast<unsigned int>(storageInfo.size)));

	StoredCredentialIssuer record{};
	size_t outputLength{ 0 };
	status = psa_ps_get(GetUid(index), 0, sizeof(record), &record, &outputLength);
	VerifyOrReturnValue(status == PSA_SUCCESS, ALIRO_ERROR_INTERNAL,
			    LOG_ERR("Failed to read Certificate Credential Issuer key (psa=%d)", status));
	VerifyOrReturnValue(outputLength == sizeof(record), ALIRO_ERROR_INTERNAL,
			    LOG_ERR("Invalid Certificate Credential Issuer record length: %u",
				    static_cast<unsigned int>(outputLength)));
	VerifyOrReturnValue(record.mVersion == StoredCredentialIssuer::kVersion, ALIRO_ERROR_INTERNAL,
			    LOG_ERR("Unsupported Certificate Credential Issuer record version: %u", record.mVersion));

	publicKey = record.mPublicKey;
	if (validityPeriod) {
		*validityPeriod = record.mValidityPeriod;
	}

	return ALIRO_NO_ERROR;
}

AliroError ClearCertificateCredentialIssuerKey(size_t index)
{
	VerifyOrReturnValue(index < kMaxStoredKeys, ALIRO_INVALID_ARGUMENT,
			    LOG_ERR("Certificate Credential Issuer key index out of range"));

	const psa_status_t status = psa_ps_remove(GetUid(index));
	VerifyOrReturnValue(status == PSA_SUCCESS || status == PSA_ERROR_DOES_NOT_EXIST, ALIRO_ERROR_INTERNAL,
			    LOG_ERR("Failed to clear Certificate Credential Issuer key (psa=%d)", status));

	return ALIRO_NO_ERROR;
}

AliroError LoadCertificateCredentialIssuerKeys()
{
	CryptoTypes::PublicKey publicKey{};

	for (size_t index = 0; index < kMaxStoredKeys; index++) {
		const auto error = ReadCertificateCredentialIssuerKey(index, publicKey);
		if (error == ALIRO_PUBLIC_KEY_NOT_FOUND) {
			continue;
		}
		VerifyOrReturnValue(error == ALIRO_NO_ERROR, error,
				    LOG_ERR("Failed to load Certificate Credential Issuer key at index %zu", index));

		const AliroError addError = AccessManagerInstance().AddPublicKey(
			publicKey, AccessManager::PublicKeyType::CertificateCredentialIssuer, index);
		VerifyOrReturnValue(addError == ALIRO_NO_ERROR, addError);
	}

	return ALIRO_NO_ERROR;
}

} // namespace Aliro
