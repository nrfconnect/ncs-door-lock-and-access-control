/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <reader_storage/reader.h>

#include "cert.h"

#include <aliro/memory.h>
#include <aliro/utils.h>
#include <settings_utils/settings_utils.h>
#include <zephyr/logging/log.h>

#include <algorithm>
#include <array>
#include <memory>
#include <optional>

LOG_MODULE_DECLARE(reader_storage, CONFIG_DOOR_LOCK_ALIRO_READER_STORAGE_LOG_LEVEL);

#define SETTINGS_KEY_NAME(name) CONFIG_DOOR_LOCK_ALIRO_READER_STORAGE_SETTINGS_BASE_KEY_NAME "/" name

namespace DoorLock::ReaderStorage {

namespace {

using namespace Aliro;

constexpr char kSettingsKeyNameReaderCertificate[]{ SETTINGS_KEY_NAME("cert") };
constexpr char kSettingsKeyNameReaderSystemIssuerCAPublicKey[]{ SETTINGS_KEY_NAME("issuer") };

std::unique_ptr<uint8_t[]> sCertificate{};
size_t sCertificateLength{ 0 };
std::optional<Aliro::CryptoTypes::PublicKey> sIssuerPublicKey{};

void ClearCertificateCache()
{
	sCertificate.reset();
	sCertificateLength = 0;
}

AliroError SetCertificateCache(const uint8_t *certificate, size_t certificateLength)
{
	VerifyOrReturnStatus(certificate, ALIRO_INVALID_ARGUMENT, LOG_ERR("Certificate data is null"));
	VerifyOrReturnStatus(IN_RANGE(certificateLength, 1, kMaxCertificateSize), ALIRO_ERROR_INTERNAL,
			     LOG_ERR("Invalid Reader certificate length: %zu (max: %zu)", certificateLength,
				     kMaxCertificateSize));

	ClearCertificateCache();

	sCertificate = Aliro::make_unique_array_nothrow<uint8_t>(certificateLength);
	VerifyOrReturnStatus(sCertificate, ALIRO_NO_MEMORY, LOG_ERR("Failed to allocate memory for certificate"));

	std::copy_n(certificate, certificateLength, sCertificate.get());
	sCertificateLength = certificateLength;

	return ALIRO_NO_ERROR;
}

AliroError LoadReaderCertificate()
{
	std::array<uint8_t, kMaxCertificateSize> certData{};
	size_t certLength{ certData.size() };

	const auto ec = SettingsUtils::Load(kSettingsKeyNameReaderCertificate, certData.data(), certLength);
	if (ec == -ENOENT) {
		LOG_WRN("Reader certificate is not set");
		return ALIRO_NO_ERROR;
	}

	VerifyOrReturnValue(ec == 0, ALIRO_ERROR_INTERNAL,
			    LOG_ERR("Cannot get Reader certificate, error code: %d", ec));

	AliroError err = SetCertificateCache(certData.data(), certLength);
	VerifyOrReturnStatus(err == ALIRO_NO_ERROR, err, LOG_ERR("Cannot set Reader certificate."));

	LOG_INF("Loaded Reader certificate: %zu bytes", certLength);
	return ALIRO_NO_ERROR;
}

AliroError LoadIssuerPublicKey()
{
	CryptoTypes::PublicKey publicKey{};
	int ec = SettingsUtils::Load(kSettingsKeyNameReaderSystemIssuerCAPublicKey, publicKey);
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

} // namespace

AliroError LoadCert()
{
	ReturnErrorOnFailure(LoadReaderCertificate());
	ReturnErrorOnFailure(LoadIssuerPublicKey());

	return ALIRO_NO_ERROR;
}

AliroError ClearCert()
{
	ReturnErrorOnFailure(ClearCertificate());
	ReturnErrorOnFailure(ClearIssuerPublicKey());

	return ALIRO_NO_ERROR;
}

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
	VerifyOrReturnStatus(certificate, ALIRO_INVALID_ARGUMENT, LOG_ERR("Certificate data is null"));
	VerifyOrReturnStatus(IN_RANGE(certificateLength, 1, kMaxCertificateSize), ALIRO_INVALID_ARGUMENT,
			     LOG_ERR("Invalid Reader certificate length: %zu (max: %zu)", certificateLength,
				     kMaxCertificateSize));

	auto error = SettingsUtils::Save(kSettingsKeyNameReaderCertificate, certificate, certificateLength);
	VerifyOrReturnValue(error == 0, ALIRO_ERROR_INTERNAL, LOG_ERR("Failed to save Reader certificate"));

	return SetCertificateCache(certificate, certificateLength);
}

AliroError ClearCertificate()
{
	ClearCertificateCache();

	const auto error = SettingsUtils::Delete(kSettingsKeyNameReaderCertificate);
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

	const auto error = SettingsUtils::Save(kSettingsKeyNameReaderSystemIssuerCAPublicKey, publicKey);
	VerifyOrReturnValue(error == 0, ALIRO_ERROR_INTERNAL,
			    LOG_ERR("Failed to save Reader System Issuer CA public key"));

	sIssuerPublicKey = publicKey;

	return ALIRO_NO_ERROR;
}

AliroError ClearIssuerPublicKey()
{
	sIssuerPublicKey.reset();

	const auto error = SettingsUtils::Delete(kSettingsKeyNameReaderSystemIssuerCAPublicKey);
	VerifyOrReturnValue(error == 0, ALIRO_ERROR_INTERNAL,
			    LOG_ERR("Failed to clear Reader System Issuer CA public key"));

	return ALIRO_NO_ERROR;
}

} // namespace DoorLock::ReaderStorage
