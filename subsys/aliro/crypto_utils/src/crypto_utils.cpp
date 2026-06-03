/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <crypto_utils/crypto_utils.h>

#include <aliro/utils.h>

#include <psa/crypto.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(crypto_utils, CONFIG_DOOR_LOCK_ALIRO_CRYPTO_UTILS_LOG_LEVEL);

using namespace Aliro;

namespace DoorLock::CryptoUtils {

namespace {

psa_key_attributes_t GetPrivateKeyAttributes()
{
	psa_key_attributes_t attributes = PSA_KEY_ATTRIBUTES_INIT;

	psa_set_key_type(&attributes, PSA_KEY_TYPE_ECC_KEY_PAIR(PSA_ECC_FAMILY_SECP_R1));
	psa_set_key_algorithm(&attributes, PSA_ALG_ECDSA(PSA_ALG_SHA_256));
	psa_set_key_bits(&attributes, PSA_BYTES_TO_BITS(CryptoTypes::kEccP256KeyPrivateKeyLength));
	psa_set_key_usage_flags(&attributes, PSA_KEY_USAGE_SIGN_HASH);

	return attributes;
}

psa_key_attributes_t GetPublicKeyAttributes()
{
	psa_key_attributes_t attributes = PSA_KEY_ATTRIBUTES_INIT;

	psa_set_key_type(&attributes, PSA_KEY_TYPE_ECC_PUBLIC_KEY(PSA_ECC_FAMILY_SECP_R1));
	psa_set_key_algorithm(&attributes, PSA_ALG_ECDSA(PSA_ALG_SHA_256));
	psa_set_key_bits(&attributes, PSA_BYTES_TO_BITS(CryptoTypes::kEccP256KeyPrivateKeyLength));
	psa_set_key_usage_flags(&attributes, PSA_KEY_USAGE_VERIFY_HASH);

	return attributes;
}

psa_key_attributes_t GetGroupResolvingKeyAttributes()
{
	psa_key_attributes_t attributes = PSA_KEY_ATTRIBUTES_INIT;

	psa_set_key_type(&attributes, PSA_KEY_TYPE_AES);
	psa_set_key_algorithm(&attributes, PSA_ALG_ECB_NO_PADDING);
	psa_set_key_bits(&attributes, PSA_BYTES_TO_BITS(CryptoTypes::kGroupResolvingKeyLength));
	psa_set_key_usage_flags(&attributes, PSA_KEY_USAGE_ENCRYPT | PSA_KEY_USAGE_DECRYPT | PSA_KEY_USAGE_EXPORT);

	return attributes;
}

AliroError SetPersistentLifetime(psa_key_attributes_t &attributes, CryptoTypes::KeyId keyId)
{
	VerifyOrReturnStatus(IN_RANGE(keyId, PSA_KEY_ID_USER_MIN, PSA_KEY_ID_USER_MAX), ALIRO_INVALID_ARGUMENT,
			     LOG_WRN("Key ID is not defined"));
	psa_set_key_id(&attributes, keyId);

	return ALIRO_NO_ERROR;
}

AliroError ImportKey(const uint8_t *key, size_t keyLength, psa_key_attributes_t &attributes, bool persistent,
		     CryptoTypes::KeyId &keyId)
{
	if (persistent) {
		ReturnErrorOnFailure(SetPersistentLifetime(attributes, keyId));
	}

	const auto status = psa_import_key(&attributes, key, keyLength, &keyId);
	VerifyOrReturnStatus(status == PSA_SUCCESS, ALIRO_ERROR_INTERNAL,
			     LOG_WRN("Could not import key [Error: %d]", status));

	return ALIRO_NO_ERROR;
}

} // namespace

AliroError Init()
{
	/* Initialize PSA Crypto */
	psa_status_t status = psa_crypto_init();
	VerifyOrReturnStatus(status == PSA_SUCCESS, ALIRO_ERROR_INTERNAL,
			     LOG_ERR("Cannot initialize PSA Crypto! (Error: %d)", status));

	return ALIRO_NO_ERROR;
}

AliroError ImportPrivateKey(const CryptoTypes::PrivateKey &privateKey, bool persistent, CryptoTypes::KeyId &keyId)
{
	auto attributes = GetPrivateKeyAttributes();
	return ImportKey(privateKey.data(), privateKey.size(), attributes, persistent, keyId);
}

AliroError ImportPublicKey(const CryptoTypes::PublicKey &publicKey, bool persistent, CryptoTypes::KeyId &keyId)
{
	auto attributes = GetPublicKeyAttributes();
	return ImportKey(publicKey.data(), publicKey.size(), attributes, persistent, keyId);
}

AliroError ImportGroupResolvingKey(const CryptoTypes::GroupResolvingKey &groupResolvingKey, bool persistent,
				   CryptoTypes::KeyId &keyId)
{
	auto attributes = GetGroupResolvingKeyAttributes();
	return ImportKey(groupResolvingKey.data(), groupResolvingKey.size(), attributes, persistent, keyId);
}

AliroError ExportPublicKey(CryptoTypes::KeyId keyId, CryptoTypes::PublicKey &publicKey)
{
	size_t outputLength{};
	psa_status_t status = psa_export_public_key(keyId, publicKey.data(), publicKey.size(), &outputLength);

	VerifyOrReturnStatus(status == PSA_SUCCESS, ALIRO_PUBLIC_KEY_NOT_FOUND,
			     LOG_WRN("Cannot export public key with ID: 0x%x [Error: %d]", keyId, status));
	VerifyOrReturnStatus(outputLength == publicKey.size(), ALIRO_INVALID_ARGUMENT,
			     LOG_WRN("Invalid public key length"));

	return ALIRO_NO_ERROR;
}

AliroError ExportKey(CryptoTypes::KeyId keyId, uint8_t *key, size_t keyLength)
{
	VerifyOrReturnStatus(keyId != PSA_KEY_ID_NULL, ALIRO_INVALID_ARGUMENT, LOG_WRN("Invalid Key ID"));

	size_t outputLength{};
	psa_status_t status = psa_export_key(keyId, key, keyLength, &outputLength);

	VerifyOrReturnStatus(status == PSA_SUCCESS, ALIRO_ERROR_INTERNAL,
			     LOG_WRN("Cannot export key ID: 0x%x [Error: %d]", keyId, status));
	VerifyOrReturnStatus(outputLength == keyLength, ALIRO_ERROR_INTERNAL, LOG_WRN("Invalid key length"));

	return ALIRO_NO_ERROR;
}

AliroError PreserveKey(CryptoTypes::KeyId volatileKeyId, CryptoTypes::KeyId persistentKeyId)
{
	// Validate destination key ID range
	VerifyOrReturnStatus(IN_RANGE(persistentKeyId, PSA_KEY_ID_USER_MIN, PSA_KEY_ID_USER_MAX),
			     ALIRO_INVALID_ARGUMENT,
			     LOG_WRN("Destination key ID 0x%x is outside valid range [0x%x, 0x%x]", persistentKeyId,
				     PSA_KEY_ID_USER_MIN, PSA_KEY_ID_USER_MAX));

	psa_key_attributes_t attributes{};
	psa_status_t status = psa_get_key_attributes(volatileKeyId, &attributes);
	VerifyOrReturnStatus(status == PSA_SUCCESS, ALIRO_ERROR_INTERNAL,
			     LOG_DBG("Cannot get key attributes [Error: %d]", status));

	psa_set_key_id(&attributes, persistentKeyId);

	CryptoTypes::KeyId keyId{};
	status = psa_copy_key(volatileKeyId, &attributes, &keyId);
	if (status == PSA_ERROR_ALREADY_EXISTS) {
		LOG_INF("Key already exists [Status: %d]", status);
		return ALIRO_KEY_ALREADY_EXISTS;
	}

	VerifyOrReturnStatus(status == PSA_SUCCESS, ALIRO_ERROR_INTERNAL,
			     LOG_WRN("Cannot copy key [Error: %d]", status));

	VerifyOrReturnStatus(keyId == persistentKeyId, ALIRO_ERROR_INTERNAL, LOG_WRN("Bad key ID"));

	return ALIRO_NO_ERROR;
}

AliroError DestroyKey(CryptoTypes::KeyId &keyId)
{
	psa_status_t status = psa_destroy_key(keyId);
	AliroError error = (status == PSA_SUCCESS) ? ALIRO_NO_ERROR : ALIRO_ERROR_INTERNAL;
	VerifyOrReturnStatus(error == ALIRO_NO_ERROR, error, LOG_ERR("Cannot destroy key [Error: %d]", status));
	keyId = 0;

	return error;
}

AliroError IsKeyAvailable(CryptoTypes::KeyId keyId)
{
	VerifyOrReturnStatus(keyId != PSA_KEY_ID_NULL, ALIRO_INVALID_ARGUMENT, LOG_WRN("Key ID is not defined"));

	psa_key_attributes_t attributes = PSA_KEY_ATTRIBUTES_INIT;
	psa_status_t status = psa_get_key_attributes(keyId, &attributes);
	VerifyOrReturnStatus(status == PSA_SUCCESS, ALIRO_ERROR_INTERNAL,
			     LOG_DBG("Cannot get key attributes [Error: %d]", status));

	return ALIRO_NO_ERROR;
}

AliroError VerifySignature(CryptoTypes::KeyId keyId, const uint8_t *msg, const size_t msgLength,
			   const CryptoTypes::Signature &signature)
{
	VerifyOrReturnStatus(msg && msgLength > 0, ALIRO_INVALID_ARGUMENT);

	psa_status_t ec = psa_verify_message(keyId, PSA_ALG_ECDSA(PSA_ALG_SHA_256), msg, msgLength, signature.data(),
					     signature.size());

	return (ec == PSA_SUCCESS) ? ALIRO_NO_ERROR : ALIRO_INVALID_SIGNATURE;
}

} // namespace DoorLock::CryptoUtils
