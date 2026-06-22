/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include <aliro/errors.h>
#include <aliro/types.h>

namespace DoorLock::CryptoUtils {

/**
 * @brief Initialize the crypto module.
 *
 * @return ALIRO_NO_ERROR on success, error status otherwise.
 */
AliroError Init();

/**
 * @brief Import a private key.
 *
 * @param privateKey input buffer with the private key.
 * @param persistent true if the key is persistent, false otherwise.
 * @param keyId output identifier of the imported key. Has to be set to a valid key ID if the key is persistent.
 *
 * @return ALIRO_NO_ERROR on success, error status otherwise.
 */
AliroError ImportPrivateKey(const Aliro::CryptoTypes::PrivateKey &privateKey, bool persistent,
			    Aliro::CryptoTypes::KeyId &keyId);

/**
 * @brief Import a public key.
 *
 * @param publicKey input buffer with the public key.
 * @param persistent true if the key is persistent, false otherwise.
 * @param keyId output identifier of the imported key. Has to be set to a valid key ID if the key is persistent.
 *
 * @return ALIRO_NO_ERROR on success, error status otherwise.
 */
AliroError ImportPublicKey(const Aliro::CryptoTypes::PublicKey &publicKey, bool persistent,
			   Aliro::CryptoTypes::KeyId &keyId);

/**
 * @brief Import a Group Resolving Key.
 *
 * @param groupResolvingKey input buffer with the Group Resolving Key.
 * @param persistent true if the key is persistent, false otherwise.
 * @param keyId output identifier of the imported key. Has to be set to a valid key ID if the key is persistent.
 *
 * @return ALIRO_NO_ERROR on success, error status otherwise.
 */
AliroError ImportGroupResolvingKey(const Aliro::CryptoTypes::GroupResolvingKey &groupResolvingKey, bool persistent,
				   Aliro::CryptoTypes::KeyId &keyId);

/**
 * @brief Export EC public key.
 *
 * @param keyId input identifier of a private key for which the public key should be exported.
 * @param publicKey Output buffer where the public key is to be copied.
 *
 * @return ALIRO_NO_ERROR on success, error status otherwise.
 */
AliroError ExportPublicKey(Aliro::CryptoTypes::KeyId keyId, Aliro::CryptoTypes::PublicKey &publicKey);

/**
 * @brief Export a key.
 *
 * @note The caller must know the key type associated with the given keyId.
 * The size of the buffer must match the key size for that type.
 *
 * @param keyId input identifier of a key to export.
 * @param key output buffer where the key is to be copied.
 * @param keyLength length of the key in bytes.
 *
 * @return ALIRO_NO_ERROR on success, error status otherwise.
 */
AliroError ExportKey(Aliro::CryptoTypes::KeyId keyId, uint8_t *key, size_t keyLength);

/**
 * @brief Preserve a volatile key by copying it to persistent slot.
 *
 * @param volatileKeyId input identifier of the volatile key.
 * @param persistentKeyId input identifier of the persistent key.
 *
 * @return ALIRO_NO_ERROR on success, ALIRO_KEY_ALREADY_EXISTS if the key already exists, other error status
 * otherwise.
 */
AliroError PreserveKey(Aliro::CryptoTypes::KeyId volatileKeyId, Aliro::CryptoTypes::KeyId persistentKeyId);

/**
 * @brief Destroy an key by ID.
 *
 * @note When the key is successfully destroyed, keyId is set to 0.
 *
 * @param keyId identifier of a key to delete.
 *
 * @return ALIRO_NO_ERROR on success, error status otherwise.
 */
AliroError DestroyKey(Aliro::CryptoTypes::KeyId &keyId);

/**
 * @brief Checks if a key ID exists.
 *
 * @param keyId The key ID to check.
 *
 * @return ALIRO_NO_ERROR if the key is available, a error code otherwise.
 */
AliroError IsKeyAvailable(Aliro::CryptoTypes::KeyId keyId);

/**
 * @brief Verify signature of a message.
 *
 * @param keyId input identifier of the public key to use for verification.
 * @param msg input message whose signature is to be verified.
 * @param msgLength input size of the message.
 * @param signature input signature to verify.
 *
 * @return ALIRO_NO_ERROR when signature is valid, ALIRO_INVALID_SIGNATURE otherwise.
 */
AliroError VerifySignature(Aliro::CryptoTypes::KeyId keyId, const uint8_t *msg, const size_t msgLength,
			   const Aliro::CryptoTypes::Signature &signature);

} // namespace DoorLock::CryptoUtils
