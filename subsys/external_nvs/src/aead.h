/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

/**
 * @file aead.h
 * @brief AEAD encryption and decryption for External NVS.
 *
 * Provides authenticated encryption with associated data (AEAD) using the
 * ChaCha20-Poly1305 algorithm via the PSA Crypto driver interface. The
 * authentication tag is appended to the ciphertext on encryption and
 * verified during decryption.
 */

#pragma once

#include "key.h"
#include "nonce.h"

#include <psa/crypto.h>

#include <cstddef>
#include <cstdint>

namespace DoorLock::ExternalNvs::Aead {

/**
 * @brief Get the maximum ciphertext size for a given plaintext size.
 *
 * @param plaintextSize The size of the plaintext in bytes.
 *
 * @return The maximum ciphertext size in bytes.
 */
constexpr size_t GetMaxCiphertextSize(size_t plaintextSize)
{
	return PSA_AEAD_ENCRYPT_OUTPUT_MAX_SIZE(plaintextSize);
}

/**
 * @brief Type representing a ciphertext buffer.
 *
 * @tparam kPlaintextSize The size of the plaintext in bytes.
 */
template <size_t kPlaintextSize> using Ciphertext = std::array<uint8_t, GetMaxCiphertextSize(kPlaintextSize)>;

/**
 * @brief Encrypt plaintext with AEAD (ChaCha20-Poly1305).
 *
 * Encrypts @p plainText and appends a Poly1305 authentication tag.
 * The @p aad (additional authenticated data) is authenticated but not
 * encrypted.
 *
 * @param      key              Encryption key (256 bits).
 * @param      nonce            Nonce (96 bits). Must be unique per key usage.
 * @param      aad              Pointer to additional authenticated data (may be @c nullptr if
 *                              @p aadLength is 0).
 * @param      aadLength        Length of @p aad in bytes.
 * @param      plainText        Pointer to the plaintext to encrypt.
 * @param      plainTextLength  Length of @p plainText in bytes.
 * @param[out] cipherText       Buffer that receives the ciphertext and appended tag.
 * @param[in,out] cipherTextLength On input, the capacity of @p cipherText in bytes.
 *                              On output, the number of bytes written (ciphertext + tag).
 *
 * @retval 0 on success.
 * @retval -EIO if the encryption operation fails.
 */
int Encrypt(const Key::Key &key, const Nonce::Nonce &nonce, const uint8_t *aad, size_t aadLength,
	    const uint8_t *plainText, size_t plainTextLength, uint8_t *cipherText, size_t &cipherTextLength);

/**
 * @brief Decrypt ciphertext with AEAD (ChaCha20-Poly1305).
 *
 * Verifies the Poly1305 authentication tag and decrypts @p cipherText.
 * The @p aad must match the data used during encryption for the tag
 * verification to succeed.
 *
 * @param      key               Decryption key (256 bits).
 * @param      nonce             Nonce (96 bits) used during encryption.
 * @param      aad               Pointer to additional authenticated data (may be @c nullptr if
 *                               @p aadLength is 0).
 * @param      aadLength         Length of @p aad in bytes.
 * @param      cipherText        Pointer to the ciphertext with appended tag.
 * @param      cipherTextLength  Length of @p cipherText in bytes (including tag).
 * @param[out] plainText         Buffer that receives the decrypted plaintext.
 * @param[in,out] plainTextLength On input, the capacity of @p plainText in bytes.
 *                               On output, the number of decrypted bytes written.
 *
 * @retval 0 on success.
 * @retval -EIO if the decryption or tag verification fails.
 */
int Decrypt(const Key::Key &key, const Nonce::Nonce &nonce, const uint8_t *aad, size_t aadLength,
	    const uint8_t *cipherText, size_t cipherTextLength, uint8_t *plainText, size_t &plainTextLength);

} // namespace DoorLock::ExternalNvs::Aead
