/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "aead.h"

#include <psa/crypto.h>

extern "C" {
#include <psa_crypto_driver_wrappers.h>
}

#include <zephyr/logging/log.h>

LOG_MODULE_DECLARE(external_nvs);

namespace DoorLock::ExternalNvs::Aead {

namespace {

psa_key_attributes_t GetEncryptionKeyAttributes()
{
	psa_key_attributes_t attributes = PSA_KEY_ATTRIBUTES_INIT;

	psa_set_key_usage_flags(&attributes, PSA_KEY_USAGE_ENCRYPT);
	psa_set_key_lifetime(&attributes, PSA_KEY_LIFETIME_VOLATILE);
	psa_set_key_algorithm(&attributes, PSA_ALG_CHACHA20_POLY1305);
	psa_set_key_type(&attributes, PSA_KEY_TYPE_CHACHA20);
	psa_set_key_bits(&attributes, PSA_BYTES_TO_BITS(Key::kKeySize));

	return attributes;
}

psa_key_attributes_t GetDecryptionKeyAttributes()
{
	psa_key_attributes_t attributes = PSA_KEY_ATTRIBUTES_INIT;

	psa_set_key_usage_flags(&attributes, PSA_KEY_USAGE_DECRYPT);
	psa_set_key_lifetime(&attributes, PSA_KEY_LIFETIME_VOLATILE);
	psa_set_key_algorithm(&attributes, PSA_ALG_CHACHA20_POLY1305);
	psa_set_key_type(&attributes, PSA_KEY_TYPE_CHACHA20);
	psa_set_key_bits(&attributes, PSA_BYTES_TO_BITS(Key::kKeySize));

	return attributes;
}

} // namespace

int Encrypt(const Key::Key &key, const Nonce::Nonce &nonce, const uint8_t *aad, size_t aadLength,
	    const uint8_t *plainText, size_t plainTextLength, uint8_t *cipherText, size_t &cipherTextLength)
{
	const auto keyAttributes = GetEncryptionKeyAttributes();

	const auto status =
		psa_driver_wrapper_aead_encrypt(&keyAttributes, key.data(), key.size(), PSA_ALG_CHACHA20_POLY1305,
						nonce.data(), nonce.size(), aad, aadLength, plainText, plainTextLength,
						cipherText, cipherTextLength, &cipherTextLength);
	if (status != PSA_SUCCESS) {
		LOG_ERR("Failed to encrypt: %d", status);
		return -EIO;
	}

	return 0;
}

int Decrypt(const Key::Key &key, const Nonce::Nonce &nonce, const uint8_t *aad, size_t aadLength,
	    const uint8_t *cipherText, size_t cipherTextLength, uint8_t *plainText, size_t &plainTextLength)
{
	const auto keyAttributes = GetDecryptionKeyAttributes();

	const auto status =
		psa_driver_wrapper_aead_decrypt(&keyAttributes, key.data(), key.size(), PSA_ALG_CHACHA20_POLY1305,
						nonce.data(), nonce.size(), aad, aadLength, cipherText,
						cipherTextLength, plainText, plainTextLength, &plainTextLength);
	if (status != PSA_SUCCESS) {
		LOG_ERR("Failed to decrypt: %d", status);
		return -EIO;
	}

	return 0;
}

} // namespace DoorLock::ExternalNvs::Aead
