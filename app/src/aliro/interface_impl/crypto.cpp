/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "aliro/errors.h"
#include "aliro/interface.h"

#include "aliro/storage/psa_key_ids.h"
#include "aliro/utils.h"

#include <crypto_utils/crypto_utils.h>

#include <psa/crypto.h>
#include <zephyr/logging/log.h>

#include <algorithm>
#include <cstdio>

LOG_MODULE_REGISTER(crypto_psa, CONFIG_DOOR_LOCK_APP_LOG_LEVEL);

namespace Aliro::Interface::Crypto {

namespace {

psa_key_attributes_t GetSharedKeyAttributes()
{
	psa_key_attributes_t attributes = PSA_KEY_ATTRIBUTES_INIT;

	psa_set_key_type(&attributes, PSA_KEY_TYPE_DERIVE);
	psa_set_key_algorithm(&attributes, PSA_ALG_HKDF(PSA_ALG_SHA_256));
	psa_set_key_bits(&attributes, PSA_BYTES_TO_BITS(CryptoTypes::kEccP256KeyPrivateKeyLength));
	psa_set_key_usage_flags(&attributes, PSA_KEY_USAGE_DERIVE | PSA_KEY_USAGE_COPY);

	return attributes;
}

psa_key_attributes_t GetSymmetricKeyAttributes()
{
	psa_key_attributes_t attributes = PSA_KEY_ATTRIBUTES_INIT;

	psa_set_key_type(&attributes, PSA_KEY_TYPE_AES);
	psa_set_key_algorithm(&attributes, PSA_ALG_GCM);
	psa_set_key_bits(&attributes, PSA_BYTES_TO_BITS(CryptoTypes::kSymmetricKeyLength));
	psa_set_key_usage_flags(&attributes, PSA_KEY_USAGE_ENCRYPT | PSA_KEY_USAGE_DECRYPT);

	return attributes;
}

psa_key_attributes_t GetEphemeralKeyAttributes()
{
	psa_key_attributes_t attributes = PSA_KEY_ATTRIBUTES_INIT;

	psa_set_key_type(&attributes, PSA_KEY_TYPE_ECC_KEY_PAIR(PSA_ECC_FAMILY_SECP_R1));
	psa_set_key_algorithm(&attributes, PSA_ALG_ECDH);
	psa_set_key_bits(&attributes, PSA_BYTES_TO_BITS(CryptoTypes::kEccP256KeyPrivateKeyLength));
	psa_set_key_usage_flags(&attributes, PSA_KEY_USAGE_DERIVE);

	return attributes;
}

psa_key_attributes_t GetRawKeyAttributes(size_t outputKeyLength)
{
	psa_key_attributes_t attributes = PSA_KEY_ATTRIBUTES_INIT;

	psa_set_key_type(&attributes, PSA_KEY_TYPE_DERIVE);
	psa_set_key_algorithm(&attributes, PSA_ALG_HKDF(PSA_ALG_SHA_256));
	psa_set_key_bits(&attributes, PSA_BYTES_TO_BITS(outputKeyLength));
	psa_set_key_usage_flags(&attributes, PSA_KEY_USAGE_EXPORT);

	return attributes;
}

AliroError DeriveKey(CryptoTypes::KeyId inputKeyId, const uint8_t *info, size_t infoLength, const uint8_t *salt,
		     size_t saltLength, const psa_key_attributes_t &attributes, CryptoTypes::KeyId &outputKeyId)
{
	psa_key_derivation_operation_t operation = PSA_KEY_DERIVATION_OPERATION_INIT;

	// Set derivation algorithm.
	psa_status_t status = psa_key_derivation_setup(&operation, PSA_ALG_HKDF(PSA_ALG_SHA_256));
	VerifyOrExit(status == PSA_SUCCESS, LOG_WRN("Cannot setup key derivation [Error: %d]", status));

	// Set salt for the operation.
	status = psa_key_derivation_input_bytes(&operation, PSA_KEY_DERIVATION_INPUT_SALT, salt, saltLength);
	VerifyOrExit(status == PSA_SUCCESS, LOG_WRN("Cannot import salt [Error: %d]", status));

	// Set a key for the operation.
	status = psa_key_derivation_input_key(&operation, PSA_KEY_DERIVATION_INPUT_SECRET, inputKeyId);
	VerifyOrExit(status == PSA_SUCCESS, LOG_WRN("Cannot set input key id [Error: %d]", status));

	// Set an additional info for the operation.
	status = psa_key_derivation_input_bytes(&operation, PSA_KEY_DERIVATION_INPUT_INFO, info, infoLength);
	VerifyOrExit(status == PSA_SUCCESS, LOG_WRN("Cannot import info [Error: %d]", status));

	status = psa_key_derivation_output_key(&attributes, &operation, &outputKeyId);
	VerifyOrExit(status == PSA_SUCCESS, LOG_WRN("Cannot output derived key [Error: %d]", status));

exit:
	/* From PSA API spec: Key derivation does not finish in the same way as other multi-part
	 * operations. Call psa_key_derivation_abort() to release the key derivation operation
	 * memory when the object is no longer required. */
	psa_status_t abortStatus = psa_key_derivation_abort(&operation);

	VerifyOrReturnStatus(abortStatus == PSA_SUCCESS, ALIRO_ERROR_INTERNAL,
			     LOG_WRN("Key derivation abort failed [Error: %d]", status));

	return (status == PSA_SUCCESS) ? ALIRO_NO_ERROR : ALIRO_ERROR_INTERNAL;
}

} // namespace

AliroError GenerateRandom(uint8_t *buffer, size_t bufferLength)
{
	VerifyOrReturnStatus(buffer && bufferLength > 0, ALIRO_INVALID_ARGUMENT);

	return psa_generate_random(buffer, bufferLength) == PSA_SUCCESS ? ALIRO_NO_ERROR : ALIRO_ERROR_INTERNAL;
}

AliroError GenerateEphemeralKeyPair(CryptoTypes::KeyId &keyId, CryptoTypes::PublicKey &ephemeralPubKey)
{
	const auto attributes = GetEphemeralKeyAttributes();

	auto status = psa_generate_key(&attributes, &keyId);
	VerifyOrReturnStatus(status == PSA_SUCCESS, ALIRO_ERROR_INTERNAL,
			     LOG_WRN("Cannot generate ephemeral keys [error: %d]", status));

	return DoorLock::CryptoUtils::ExportPublicKey(keyId, ephemeralPubKey);
}

AliroError ImportSharedKey(const uint8_t *key, size_t keyLength, CryptoTypes::KeyId &keyId)
{
	VerifyOrReturnStatus(key && keyLength > 0, ALIRO_INVALID_ARGUMENT);

	const auto attributes = GetSharedKeyAttributes();

	psa_status_t status = psa_import_key(&attributes, key, keyLength, &keyId);
	VerifyOrReturnStatus(status == PSA_SUCCESS, ALIRO_ERROR_INTERNAL,
			     LOG_WRN("Could not import key [Error: %d]", status));

	return ALIRO_NO_ERROR;
}

AliroError ImportSymmetricKey(const uint8_t *key, size_t keyLength, CryptoTypes::KeyId &keyId)
{
	VerifyOrReturnStatus(key && keyLength > 0, ALIRO_INVALID_ARGUMENT);

	const auto attributes = GetSymmetricKeyAttributes();

	psa_status_t status = psa_import_key(&attributes, key, keyLength, &keyId);
	VerifyOrReturnStatus(status == PSA_SUCCESS, ALIRO_ERROR_INTERNAL,
			     LOG_WRN("Could not import key [Error: %d]", status));

	return ALIRO_NO_ERROR;
}

AliroError DestroyKey(CryptoTypes::KeyId &keyId)
{
	return DoorLock::CryptoUtils::DestroyKey(keyId);
}

AliroError GenerateSignature(const uint8_t *msg, const size_t msgLength, CryptoTypes::Signature &signature)
{
	VerifyOrReturnStatus(msg && msgLength > 0, ALIRO_INVALID_ARGUMENT);

	psa_status_t status = PSA_SUCCESS;
	size_t outputLen{};

	status = psa_sign_message(DoorLock::Storage::PsaKeyIds::kPrivateKeyId, PSA_ALG_ECDSA(PSA_ALG_SHA_256), msg,
				  msgLength, signature.data(), signature.size(), &outputLen);

	VerifyOrReturnStatus(status == PSA_SUCCESS, ALIRO_ERROR_INTERNAL,
			     LOG_WRN("Cannot sign message [Error: %d]", status));
	VerifyOrReturnStatus(outputLen == CryptoTypes::kEccP256SignatureLength, ALIRO_ERROR_INTERNAL,
			     LOG_WRN("Invalid signature length"));

	return ALIRO_NO_ERROR;
}

AliroError VerifySignature(const CryptoTypes::PublicKey &publicKey, const uint8_t *msg, const size_t msgLength,
			   const CryptoTypes::Signature &signature)
{
	CryptoTypes::KeyId pubKeyId{ 0 };
	ReturnErrorOnFailure(DoorLock::CryptoUtils::ImportPublicKey(publicKey, false, pubKeyId));

	const auto status = DoorLock::CryptoUtils::VerifySignature(pubKeyId, msg, msgLength, signature);

	DoorLock::CryptoUtils::DestroyKey(pubKeyId);

	return status;
}

AliroError RawKeyAgreement(CryptoTypes::KeyId keyId, const CryptoTypes::PublicKey &peerPublicKey,
			   CryptoTypes::SharedSecret &sharedSecret)
{
	size_t outputLength{};

	psa_status_t status = psa_raw_key_agreement(PSA_ALG_ECDH, // ECKA-DH with P-256
						    keyId, peerPublicKey.data(), peerPublicKey.size(),
						    sharedSecret.data(), sharedSecret.size(), &outputLength);
	VerifyOrReturnStatus(status == PSA_SUCCESS, ALIRO_ERROR_INTERNAL,
			     LOG_WRN("Cannot perform raw key agreement [Error: %d]", status));
	VerifyOrReturnStatus(outputLength == sharedSecret.size(), ALIRO_ERROR_INTERNAL,
			     LOG_WRN("Invalid raw key agreement output length"));

	return ALIRO_NO_ERROR;
}

AliroError DeriveSharedKey(CryptoTypes::KeyId keyId, const uint8_t *info, size_t infoLength, const uint8_t *salt,
			   size_t saltLength, CryptoTypes::KeyId &outputKeyId)
{
	const auto attributes = GetSharedKeyAttributes();

	return DeriveKey(keyId, info, infoLength, salt, saltLength, attributes, outputKeyId);
}

AliroError DeriveSymmetricKey(CryptoTypes::KeyId keyId, const uint8_t *info, size_t infoLength, const uint8_t *salt,
			      size_t saltLength, CryptoTypes::KeyId &outputKeyId)
{
	const auto attributes = GetSymmetricKeyAttributes();

	return DeriveKey(keyId, info, infoLength, salt, saltLength, attributes, outputKeyId);
}

AliroError DeriveRawKey(CryptoTypes::KeyId keyId, const uint8_t *info, size_t infoLength, const uint8_t *salt,
			size_t saltLength, uint8_t *outputKey, size_t outputKeyLength)
{
	VerifyOrReturnStatus(outputKey && outputKeyLength > 0, ALIRO_INVALID_ARGUMENT, LOG_WRN("Invalid output key"));

	const psa_key_attributes_t attributes = GetRawKeyAttributes(outputKeyLength);

	CryptoTypes::KeyId derivedKeyId{};
	AliroError status = DeriveKey(keyId, info, infoLength, salt, saltLength, attributes, derivedKeyId);
	VerifyOrExit(status == ALIRO_NO_ERROR);

	status = DoorLock::CryptoUtils::ExportKey(derivedKeyId, outputKey, outputKeyLength);

exit:
	DoorLock::CryptoUtils::DestroyKey(derivedKeyId);

	return status;
}

AliroError AeadEncrypt(CryptoTypes::KeyId keyId, const uint8_t *plainTxt, size_t plainTxtLength,
		       const uint8_t *additionalData, size_t additionalDataLength, const CryptoTypes::Nonce &nonce,
		       uint8_t *cipherText, CryptoTypes::AuthenticationTag &authTag)
{
	VerifyOrReturnStatus(nonce.size() == CryptoTypes::kNonceLength, ALIRO_INVALID_ARGUMENT);
	VerifyOrReturnStatus(((plainTxt != nullptr) == (plainTxtLength != 0)), ALIRO_ERROR_INTERNAL);
	VerifyOrReturnStatus(additionalData || additionalDataLength == 0, ALIRO_ERROR_INTERNAL);
	VerifyOrReturnStatus(cipherText, ALIRO_ERROR_INTERNAL);

	const psa_algorithm_t algorithm = PSA_ALG_GCM;

#ifndef CONFIG_DOOR_LOCK_CRYPTO_PSA_AEAD_SINGLE_PART
	psa_aead_operation_t operation = PSA_AEAD_OPERATION_INIT;
	size_t outLength{};
	size_t authTagLengthOutput{};

	psa_status_t status = psa_aead_encrypt_setup(&operation, keyId, algorithm);
	VerifyOrExit(status == PSA_SUCCESS, LOG_WRN("Cannot setup encryption [Error: %d]", status));

	status = psa_aead_set_lengths(&operation, additionalDataLength, plainTxtLength);
	VerifyOrExit(status == PSA_SUCCESS, LOG_WRN("Cannot set lengths [Error: %d]", status));

	status = psa_aead_set_nonce(&operation, nonce.data(), nonce.size());
	VerifyOrExit(status == PSA_SUCCESS, LOG_WRN("Cannot set nonce [Error: %d]", status));

	if (additionalDataLength != 0) {
		status = psa_aead_update_ad(&operation, additionalData, additionalDataLength);
		VerifyOrExit(status == PSA_SUCCESS, LOG_WRN("AEAD additional update failed [Error: %d]", status));
	}

	if (plainTxt) {
		// For now assume the plainTxt is encrypted all at once
		status = psa_aead_update(&operation, plainTxt, plainTxtLength, cipherText,
					 PSA_AEAD_UPDATE_OUTPUT_SIZE(PSA_KEY_TYPE_AES, algorithm, plainTxtLength),
					 &outLength);

		VerifyOrExit(status == PSA_SUCCESS, LOG_WRN("AEAD update failed [Error: %d]", status));

		cipherText += outLength;
	}

	status = psa_aead_finish(&operation, cipherText, PSA_AEAD_FINISH_OUTPUT_SIZE(PSA_KEY_TYPE_AES, algorithm),
				 &outLength, authTag.data(), authTag.size(), &authTagLengthOutput);

	VerifyOrExit(status == PSA_SUCCESS && authTag.size() == authTagLengthOutput,
		     LOG_WRN("Unexpected output authentication tag size or AEAD finish "
			     "failed [Error: %d]",
			     status));

	return ALIRO_NO_ERROR;

exit:
	status = psa_aead_abort(&operation);
	if (status != PSA_SUCCESS) {
		LOG_WRN("Cannot abort AEAD operation [Error: %d]", status);
	}
	return ALIRO_ERROR_INTERNAL;

#else // CONFIG_DOOR_LOCK_CRYPTO_PSA_AEAD_SINGLE_PART

	constexpr size_t kPlainTextSize{ CONFIG_DOOR_LOCK_CRYPTO_PSA_AEAD_SINGLE_PART_BUFFER_SIZE };
	constexpr size_t kBufferSize{ kPlainTextSize + CryptoTypes::kAuthenticationTagLength };
	const size_t expOutLen{ plainTxtLength + CryptoTypes::kAuthenticationTagLength };

	VerifyOrReturnValue(expOutLen <= kBufferSize, ALIRO_NO_MEMORY);

	std::array<uint8_t, kBufferSize> buffer{};
	size_t outLen{};

	psa_status_t status =
		psa_aead_encrypt(keyId, algorithm, nonce.data(), nonce.size(), additionalData, additionalDataLength,
				 plainTxt, plainTxtLength, buffer.data(), buffer.size(), &outLen);
	VerifyOrReturnStatus(status == PSA_SUCCESS && outLen == expOutLen, ALIRO_ERROR_INTERNAL,
			     LOG_WRN("AEAD encryption failed [Error: %d]", status));

	if (plainTxtLength) {
		std::copy_n(buffer.data(), plainTxtLength, cipherText);
	}

	std::copy_n(buffer.data() + plainTxtLength, CryptoTypes::kAuthenticationTagLength, authTag.data());

	return ALIRO_NO_ERROR;

#endif // CONFIG_DOOR_LOCK_CRYPTO_PSA_AEAD_SINGLE_PART
}

AliroError AeadDecrypt(CryptoTypes::KeyId keyId, const uint8_t *cipherTextWithTag, size_t cipherTextWithTagLength,
		       const uint8_t *additionalData, size_t additionalDataLength, const CryptoTypes::Nonce &nonce,
		       uint8_t *plainText, size_t &plainTextLength)
{
	size_t outLength{};

	VerifyOrReturnStatus(cipherTextWithTagLength != 0 && cipherTextWithTag, ALIRO_INVALID_ARGUMENT,
			     LOG_WRN("Cipher text with tag is not valid"));
	VerifyOrReturnStatus(plainTextLength == 0 || plainText, ALIRO_INVALID_ARGUMENT,
			     LOG_WRN("Plain text buffer is not valid"));

	psa_status_t status =
		psa_aead_decrypt(keyId, PSA_ALG_GCM, nonce.data(), nonce.size(), additionalData, additionalDataLength,
				 cipherTextWithTag, cipherTextWithTagLength, plainText, plainTextLength, &outLength);

	// The ciphertext is not authentic, authentication tag is not valid.
	if (status == PSA_ERROR_INVALID_SIGNATURE) {
		return ALIRO_INVALID_AUTHENTICATION_TAG;
	}

	VerifyOrReturnStatus(status == PSA_SUCCESS, ALIRO_ERROR_INTERNAL,
			     LOG_WRN("AEAD decryption failed [Error: %d]", status));

	plainTextLength = outLength;
	return ALIRO_NO_ERROR;
}

#ifdef CONFIG_NCS_ALIRO_BLE_UWB

AliroError Encrypt(const uint8_t *plainText, size_t plainTextLength, uint8_t *cipherText)
{
	constexpr static psa_algorithm_t algorithm = PSA_ALG_ECB_NO_PADDING;
	constexpr static size_t blockSize = PSA_BLOCK_CIPHER_BLOCK_LENGTH(PSA_KEY_TYPE_AES);

	VerifyOrReturnStatus((cipherText && plainText && plainTextLength == blockSize), ALIRO_INVALID_ARGUMENT);

	size_t outLength{};

	psa_status_t status = psa_cipher_encrypt(DoorLock::Storage::PsaKeyIds::kGroupResolvingKeyId, algorithm,
						 plainText, blockSize, cipherText, blockSize, &outLength);
	VerifyOrReturnStatus(status == PSA_SUCCESS, ALIRO_ERROR_INTERNAL,
			     LOG_WRN("Cannot encrypt payload [Error: %d]", status));
	VerifyOrReturnStatus(outLength == blockSize, ALIRO_ERROR_INTERNAL, LOG_WRN("Invalid output length"));

	return ALIRO_NO_ERROR;
}

#endif // CONFIG_NCS_ALIRO_BLE_UWB

AliroError Sha256(const uint8_t *data, size_t dataLength, CryptoTypes::Sha256Hash &hash)
{
	VerifyOrReturnStatus(data && dataLength > 0, ALIRO_INVALID_ARGUMENT);

	size_t hashLength;
	psa_status_t status =
		psa_hash_compute(PSA_ALG_SHA_256, data, dataLength, hash.data(), hash.size(), &hashLength);
	VerifyOrReturnStatus(status == PSA_SUCCESS, ALIRO_ERROR_INTERNAL,
			     LOG_WRN("Cannot compute SHA-256 hash [Error: %d]", status));
	VerifyOrReturnStatus(hashLength == hash.size(), ALIRO_ERROR_INTERNAL, LOG_WRN("Invalid SHA-256 hash length"));

	return ALIRO_NO_ERROR;
}

} // namespace Aliro::Interface::Crypto
