/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <reader_storage/reader.h>

#include "core.h"
#include "reader_psa_ps_ids.h"

#include <aliro/utils.h>
#include <crypto_utils/crypto_utils.h>
#include <zephyr/logging/log.h>

#include <optional>

LOG_MODULE_DECLARE(reader_storage, CONFIG_DOOR_LOCK_ALIRO_READER_STORAGE_LOG_LEVEL);

namespace DoorLock::ReaderStorage {

namespace {

using namespace Aliro;

std::optional<Aliro::Identifier> sIdentifier{};
std::optional<Aliro::CryptoTypes::PublicKey> sPublicKey{};

AliroError LoadPublicKeyFromPrivateKey()
{
	if (!IsPrivateKeySet()) {
		LOG_WRN("Private key is not set");
		return ALIRO_NO_ERROR;
	}

	CryptoTypes::PublicKey publicKey{};
	const auto error = CryptoUtils::ExportPublicKey(kPrivateKeyId, publicKey);
	VerifyOrReturnValue(error == ALIRO_NO_ERROR, error, LOG_ERR("Failed to export Reader public key"));

	sPublicKey = publicKey;

	return ALIRO_NO_ERROR;
}

AliroError LoadIdentifier()
{
	Identifier identifier{};
	size_t outputLength{ 0 };

	const psa_status_t status =
		psa_ps_get(kReaderIdentifierUid, 0, identifier.size(), identifier.data(), &outputLength);
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

} // namespace

AliroError LoadCore()
{
	ReturnErrorOnFailure(LoadPublicKeyFromPrivateKey());
	ReturnErrorOnFailure(LoadIdentifier());

	return ALIRO_NO_ERROR;
}

AliroError ClearCore()
{
	ReturnErrorOnFailure(ClearIdentifier());
	ReturnErrorOnFailure(ClearPrivateKey());

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
		psa_ps_set(kReaderIdentifierUid, identifier.size(), identifier.data(), PSA_STORAGE_FLAG_NONE);
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

	const psa_status_t status = psa_ps_remove(kReaderIdentifierUid);
	if (status == PSA_ERROR_DOES_NOT_EXIST) {
		return ALIRO_NO_ERROR;
	}
	VerifyOrReturnValue(status == PSA_SUCCESS, ALIRO_ERROR_INTERNAL,
			    LOG_ERR("Failed to remove Reader identifier (psa=%d)", status));

	return ALIRO_NO_ERROR;
}

bool IsPrivateKeySet()
{
	return CryptoUtils::IsKeyAvailable(kPrivateKeyId) == ALIRO_NO_ERROR;
}

psa_key_id_t GetPrivateKeyId()
{
	return kPrivateKeyId;
}

AliroError GetPublicKey(CryptoTypes::PublicKey &publicKey)
{
	VerifyOrReturnStatus(sPublicKey.has_value(), ALIRO_INVALID_STATE, LOG_ERR("Reader public key is not set"));

	publicKey = sPublicKey.value();
	return ALIRO_NO_ERROR;
}

AliroError SetPrivateKey(const CryptoTypes::PrivateKey &privateKey)
{
	CryptoTypes::KeyId keyId{ kPrivateKeyId };
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

	if (!IsPrivateKeySet()) {
		return ALIRO_NO_ERROR;
	}

	CryptoTypes::KeyId keyId{ kPrivateKeyId };
	const auto error = CryptoUtils::DestroyKey(keyId);
	VerifyOrReturnValue(error == ALIRO_NO_ERROR, error, LOG_ERR("Failed to destroy Reader private key"));

	return ALIRO_NO_ERROR;
}

} // namespace DoorLock::ReaderStorage
