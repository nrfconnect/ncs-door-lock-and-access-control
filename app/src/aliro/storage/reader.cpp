/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "reader.h"

#include "psa_key_ids.h"
#include "psa_ps_ids.h"
#include "storage.h"
#include "storage_keys.h"

#include <aliro/memory.h>
#include <aliro/utils.h>
#include <crypto_utils/crypto_utils.h>

#include <psa/protected_storage.h>
#include <zephyr/logging/log.h>

#include <algorithm>
#include <array>
#include <memory>

LOG_MODULE_REGISTER(reader, CONFIG_DOOR_LOCK_APP_LOG_LEVEL);

namespace DoorLock::Storage::Reader {
namespace {

using namespace Aliro;

std::optional<Aliro::Identifier> sIdentifier{};
std::optional<Aliro::CryptoTypes::PublicKey> sPublicKey{};

#ifdef CONFIG_DOOR_LOCK_READER_CERTIFICATE

std::unique_ptr<uint8_t[]> sCertificate{};
size_t sCertificateLength{ 0 };
std::optional<Aliro::CryptoTypes::PublicKey> sIssuerPublicKey{};

#endif // CONFIG_DOOR_LOCK_READER_CERTIFICATE

AliroError LoadPublicKeyFromPrivateKey()
{
	if (!IsPrivateKeySet()) {
		LOG_WRN("Private key is not set");
		return ALIRO_NO_ERROR;
	}

	CryptoTypes::PublicKey publicKey{};
	const auto error = CryptoUtils::ExportPublicKey(PsaKeyIds::kPrivateKeyId, publicKey);
	VerifyOrReturnValue(error == ALIRO_NO_ERROR, error, LOG_ERR("Failed to export Reader public key"));

	sPublicKey = publicKey;

	return ALIRO_NO_ERROR;
}

AliroError LoadIdentifier()
{
	Identifier identifier{};
	size_t outputLength{ 0 };

	const psa_status_t status =
		psa_ps_get(PsaPsIds::kReaderIdentifierUid, 0, identifier.size(), identifier.data(), &outputLength);
	if (status == PSA_ERROR_DOES_NOT_EXIST) {
		LOG_WRN("Reader identifier is not set");
		return ALIRO_NO_ERROR;
	}

	VerifyOrReturnValue(status == PSA_SUCCESS, ALIRO_ERROR_INTERNAL,
			    LOG_ERR("Failed to load Reader identifier (psa=%d)", status));
	VerifyOrReturnValue(outputLength == identifier.size(), ALIRO_ERROR_INTERNAL,
			    LOG_ERR("Invalid reader identifier length: %u", static_cast<unsigned int>(outputLength)));

	sIdentifier = identifier;

	return ALIRO_NO_ERROR;
}

#ifdef CONFIG_DOOR_LOCK_READER_CERTIFICATE

void ClearCertificateCache()
{
	sCertificate.reset();
	sCertificateLength = 0;
}

AliroError SetCertificateCache(const uint8_t *certificate, size_t certificateLength)
{
	VerifyOrReturnStatus(certificate, ALIRO_INVALID_ARGUMENT, LOG_ERR("Certificate data is null"));
	VerifyOrReturnStatus(IN_RANGE(certificateLength, 1, StorageKeys::kMaxCertificateSize), ALIRO_ERROR_INTERNAL,
			     LOG_ERR("Invalid Reader certificate length: %u (max: %zu)", certificateLength,
				     StorageKeys::kMaxCertificateSize));

	ClearCertificateCache();

	sCertificate = Aliro::make_unique_array_nothrow<uint8_t>(certificateLength);
	VerifyOrReturnStatus(sCertificate, ALIRO_NO_MEMORY, LOG_ERR("Failed to allocate memory for certificate"));

	std::copy_n(certificate, certificateLength, sCertificate.get());
	sCertificateLength = certificateLength;

	return ALIRO_NO_ERROR;
}

AliroError LoadReaderCertificate()
{
	uint16_t certLength{};
	int ec = KeyValueStorage::Instance().Get(StorageKeys::kStorageKeyNameReaderCertificateLength,
						 reinterpret_cast<uint8_t *>(&certLength), sizeof(certLength));
	if (ec == -ENOENT) {
		LOG_WRN("Reader certificate is not set");
		return ALIRO_NO_ERROR;
	}

	VerifyOrReturnValue(ec == 0, ALIRO_ERROR_INTERNAL,
			    LOG_ERR("Cannot get Reader certificate length, error code: %d", ec));

	VerifyOrReturnStatus(IN_RANGE(certLength, 1, StorageKeys::kMaxCertificateSize), ALIRO_ERROR_INTERNAL,
			     LOG_ERR("Invalid Reader certificate length: %u (max: %zu)", certLength,
				     StorageKeys::kMaxCertificateSize));

	std::array<uint8_t, StorageKeys::kMaxCertificateSize> certData{};
	ec = KeyValueStorage::Instance().Get(StorageKeys::kStorageKeyNameReaderCertificate, certData.data(),
					     certLength);

	VerifyOrReturnValue(ec == 0, ALIRO_ERROR_INTERNAL,
			    LOG_ERR("Cannot get Reader certificate, error code: %d", ec));

	AliroError err = SetCertificateCache(certData.data(), certLength);
	VerifyOrReturnStatus(err == ALIRO_NO_ERROR, err, LOG_ERR("Cannot set Reader certificate."));

	LOG_INF("Loaded Reader certificate: %u bytes", certLength);
	return ALIRO_NO_ERROR;
}

AliroError LoadIssuerPublicKey()
{
	CryptoTypes::PublicKey publicKey{};
	int ec = KeyValueStorage::Instance().Get(StorageKeys::kStorageKeyNameReaderSystemIssuerCAPublicKey,
						 publicKey.data(), publicKey.size());
	if (ec == -ENOENT) {
		LOG_WRN("Reader System Issuer CA public key is not set");
		return ALIRO_NO_ERROR;
	}

	VerifyOrReturnValue(ec == 0, ALIRO_ERROR_INTERNAL,
			    LOG_ERR("Cannot get Reader System Issuer CA public key, error code: %d", ec));

	VerifyOrReturnValue(publicKey[0] == CryptoTypes::kEccP256PublicKeyPrefix, ALIRO_ERROR_INTERNAL,
			    LOG_ERR("Invalid Reader System Issuer CA public key format (expected prefix 0x04)"));

	sIssuerPublicKey = publicKey;

	LOG_INF("Loaded Reader System Issuer CA public key");
	return ALIRO_NO_ERROR;
}

#endif // CONFIG_DOOR_LOCK_READER_CERTIFICATE

#if defined(CONFIG_DOOR_LOCK_BLE_UWB) && !defined(CONFIG_CHIP)

AliroError EnsureGroupResolvingKey()
{
	const bool isKeyAvailable = CryptoUtils::IsKeyAvailable(PsaKeyIds::kGroupResolvingKeyId) == ALIRO_NO_ERROR;
	if (isKeyAvailable) {
		return ALIRO_NO_ERROR;
	}

	CryptoTypes::GroupResolvingKey groupResolvingKey{};
	CryptoTypes::KeyId keyId = PsaKeyIds::kGroupResolvingKeyId;
	const auto error = CryptoUtils::ImportGroupResolvingKey(groupResolvingKey, true, keyId);
	VerifyOrReturnValue(error == ALIRO_NO_ERROR, error, LOG_ERR("Failed to import Group Resolving Key"));

	return ALIRO_NO_ERROR;
}

#endif // CONFIG_DOOR_LOCK_BLE_UWB && !CONFIG_CHIP

} // namespace

AliroError Init()
{
	ReturnErrorOnFailure(LoadPublicKeyFromPrivateKey());
	ReturnErrorOnFailure(LoadIdentifier());

#ifdef CONFIG_DOOR_LOCK_READER_CERTIFICATE
	ReturnErrorOnFailure(LoadReaderCertificate());
	ReturnErrorOnFailure(LoadIssuerPublicKey());
#endif // CONFIG_DOOR_LOCK_READER_CERTIFICATE

#if defined(CONFIG_DOOR_LOCK_BLE_UWB) && !defined(CONFIG_CHIP)
	ReturnErrorOnFailure(EnsureGroupResolvingKey());
#endif // CONFIG_DOOR_LOCK_BLE_UWB && !CONFIG_CHIP

	return ALIRO_NO_ERROR;
}

bool IsIdentifierSet()
{
	return sIdentifier.has_value();
}

AliroError GetIdentifier(Identifier &identifier)
{
	VerifyOrReturnStatus(sIdentifier.has_value(), ALIRO_INVALID_STATE, LOG_ERR("Reader identifier is not set"));

	identifier = sIdentifier.value();
	return ALIRO_NO_ERROR;
}

AliroError SetIdentifier(const Identifier &identifier)
{
	const psa_status_t status =
		psa_ps_set(PsaPsIds::kReaderIdentifierUid, identifier.size(), identifier.data(), PSA_STORAGE_FLAG_NONE);
	if (status != PSA_SUCCESS) {
		LOG_ERR("Failed to store Reader identifier (psa=%d)", status);
		return ALIRO_ERROR_INTERNAL;
	}

	sIdentifier = identifier;
	return ALIRO_NO_ERROR;
}

AliroError ClearIdentifier()
{
	sIdentifier.reset();

	const psa_status_t status = psa_ps_remove(PsaPsIds::kReaderIdentifierUid);
	if (status == PSA_ERROR_DOES_NOT_EXIST) {
		return ALIRO_NO_ERROR;
	}
	VerifyOrReturnValue(status == PSA_SUCCESS, ALIRO_ERROR_INTERNAL,
			    LOG_ERR("Failed to remove Reader identifier (psa=%d)", status));

	return ALIRO_NO_ERROR;
}

bool IsPrivateKeySet()
{
	return CryptoUtils::IsKeyAvailable(PsaKeyIds::kPrivateKeyId) == ALIRO_NO_ERROR;
}

AliroError GetPublicKey(CryptoTypes::PublicKey &publicKey)
{
	VerifyOrReturnStatus(sPublicKey.has_value(), ALIRO_INVALID_STATE, LOG_ERR("Reader public key is not set"));

	publicKey = sPublicKey.value();
	return ALIRO_NO_ERROR;
}

AliroError SetPrivateKey(const CryptoTypes::PrivateKey &privateKey)
{
	CryptoTypes::KeyId keyId{ PsaKeyIds::kPrivateKeyId };
	auto error = CryptoUtils::ImportPrivateKey(privateKey, true, keyId);
	VerifyOrReturnValue(error == ALIRO_NO_ERROR, error, LOG_ERR("Failed to import Reader private key"));

	CryptoTypes::PublicKey publicKey{};
	error = CryptoUtils::ExportPublicKey(keyId, publicKey);
	VerifyOrReturnValue(error == ALIRO_NO_ERROR, error, LOG_ERR("Failed to export Reader public key"));

	sPublicKey = publicKey;
	return ALIRO_NO_ERROR;
}

AliroError ClearPrivateKey()
{
	sPublicKey.reset();

	CryptoTypes::KeyId keyId{ PsaKeyIds::kPrivateKeyId };
	const auto error = CryptoUtils::DestroyKey(keyId);
	VerifyOrReturnValue(error == ALIRO_NO_ERROR, error, LOG_ERR("Failed to destroy Reader private key"));

	return ALIRO_NO_ERROR;
}

#ifdef CONFIG_DOOR_LOCK_READER_CERTIFICATE

bool IsCertificateSet()
{
	return sCertificate.get() != nullptr;
}

AliroError GetCertificate(ConstData &certificate)
{
	VerifyOrReturnStatus(sCertificate.get() != nullptr, ALIRO_INVALID_STATE,
			     LOG_ERR("Reader certificate is not set"));

	certificate = { sCertificate.get(), sCertificateLength };
	return ALIRO_NO_ERROR;
}

AliroError SetCertificate(const uint8_t *certificate, size_t certificateLength)
{
	VerifyOrReturnStatus(IN_RANGE(certificateLength, 1, StorageKeys::kMaxCertificateSize), ALIRO_ERROR_INTERNAL,
			     LOG_ERR("Invalid Reader certificate length: %u (max: %zu)", certificateLength,
				     StorageKeys::kMaxCertificateSize));

	auto error = KeyValueStorage::Instance().Save(StorageKeys::kStorageKeyNameReaderCertificate, certificate,
						      certificateLength);
	VerifyOrReturnValue(error == 0, ALIRO_ERROR_INTERNAL, LOG_ERR("Failed to save Reader certificate"));

	uint16_t certLength = static_cast<uint16_t>(certificateLength);
	error = KeyValueStorage::Instance().Save(StorageKeys::kStorageKeyNameReaderCertificateLength,
						 reinterpret_cast<const uint8_t *>(&certLength), sizeof(certLength));
	VerifyOrReturnValue(error == 0, ALIRO_ERROR_INTERNAL, LOG_ERR("Failed to save Reader certificate length"));

	return SetCertificateCache(certificate, certificateLength);
}

AliroError ClearCertificate()
{
	ClearCertificateCache();

	auto error = KeyValueStorage::Instance().Clear(StorageKeys::kStorageKeyNameReaderCertificateLength);
	VerifyOrReturnValue(error == 0, ALIRO_ERROR_INTERNAL, LOG_ERR("Failed to clear Reader certificate length"));

	error = KeyValueStorage::Instance().Clear(StorageKeys::kStorageKeyNameReaderCertificate);
	VerifyOrReturnValue(error == 0, ALIRO_ERROR_INTERNAL, LOG_ERR("Failed to clear Reader certificate"));

	return ALIRO_NO_ERROR;
}

bool IsIssuerPublicKeySet()
{
	return sIssuerPublicKey.has_value();
}

AliroError GetIssuerPublicKey(CryptoTypes::PublicKey &publicKey)
{
	VerifyOrReturnStatus(sIssuerPublicKey.has_value(), ALIRO_INVALID_STATE,
			     LOG_ERR("Issuer public key is not set"));

	publicKey = sIssuerPublicKey.value();
	return ALIRO_NO_ERROR;
}

AliroError SetIssuerPublicKey(const CryptoTypes::PublicKey &publicKey)
{
	VerifyOrReturnStatus(publicKey[0] == Aliro::CryptoTypes::kEccP256PublicKeyPrefix, ALIRO_INVALID_ARGUMENT,
			     LOG_ERR("Invalid Reader System Issuer CA public key prefix"));

	const auto error = KeyValueStorage::Instance().Save(StorageKeys::kStorageKeyNameReaderSystemIssuerCAPublicKey,
							    publicKey.data(), publicKey.size());
	VerifyOrReturnValue(error == 0, ALIRO_ERROR_INTERNAL,
			    LOG_ERR("Failed to save Reader System Issuer CA public key"));

	sIssuerPublicKey = publicKey;

	return ALIRO_NO_ERROR;
}

AliroError ClearIssuerPublicKey()
{
	sIssuerPublicKey.reset();

	const auto error = KeyValueStorage::Instance().Clear(StorageKeys::kStorageKeyNameReaderSystemIssuerCAPublicKey);
	VerifyOrReturnValue(error == 0, ALIRO_ERROR_INTERNAL,
			    LOG_ERR("Failed to clear Reader System Issuer CA public key"));

	return ALIRO_NO_ERROR;
}

#endif // CONFIG_DOOR_LOCK_READER_CERTIFICATE

#ifdef CONFIG_DOOR_LOCK_BLE_UWB

bool IsGroupResolvingKeySet()
{
	return CryptoUtils::IsKeyAvailable(PsaKeyIds::kGroupResolvingKeyId) == ALIRO_NO_ERROR;
}

AliroError GetGroupResolvingKey(CryptoTypes::GroupResolvingKey &groupResolvingKey)
{
	const auto error = CryptoUtils::ExportKey(PsaKeyIds::kGroupResolvingKeyId, groupResolvingKey.data(),
						  groupResolvingKey.size());
	VerifyOrReturnValue(error == ALIRO_NO_ERROR, error, LOG_ERR("Failed to export Group Resolving Key"));

	return ALIRO_NO_ERROR;
}

AliroError SetGroupResolvingKey(const CryptoTypes::GroupResolvingKey &groupResolvingKey)
{
	CryptoTypes::KeyId keyId{ PsaKeyIds::kGroupResolvingKeyId };
	const auto error = CryptoUtils::ImportGroupResolvingKey(groupResolvingKey, true, keyId);
	VerifyOrReturnValue(error == ALIRO_NO_ERROR, error, LOG_ERR("Failed to import Group Resolving Key"));

	return ALIRO_NO_ERROR;
}

AliroError ClearGroupResolvingKey()
{
	CryptoTypes::KeyId keyId{ PsaKeyIds::kGroupResolvingKeyId };
	const auto error = CryptoUtils::DestroyKey(keyId);
	VerifyOrReturnValue(error == ALIRO_NO_ERROR, error, LOG_ERR("Failed to destroy Group Resolving Key"));

	return ALIRO_NO_ERROR;
}

#endif // CONFIG_DOOR_LOCK_BLE_UWB

AliroError ClearAll()
{
	ReturnErrorOnFailure(ClearIdentifier());
	ReturnErrorOnFailure(ClearPrivateKey());

#ifdef CONFIG_DOOR_LOCK_READER_CERTIFICATE
	ReturnErrorOnFailure(ClearCertificate());
	ReturnErrorOnFailure(ClearIssuerPublicKey());
#endif // CONFIG_DOOR_LOCK_READER_CERTIFICATE

#ifdef CONFIG_DOOR_LOCK_BLE_UWB
	ReturnErrorOnFailure(ClearGroupResolvingKey());
#endif // CONFIG_DOOR_LOCK_BLE_UWB

	return ALIRO_NO_ERROR;
}

} // namespace DoorLock::Storage::Reader
