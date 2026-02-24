/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include <aliro/errors.h>
#include <aliro/types.h>

#include <cstddef>
#include <cstdint>

/**
 * @brief Reader persistent storage API.
 *
 * This namespace manages persistent storage for the Reader data:
 * - Reader identifier
 * - Reader private/public key pair
 * - Reader certificate
 * - Reader System Issuer CA public key
 * - Group Resolving Key (BLE+UWB)
 *
 * Credentials are stored persistently and cached in memory for quick access.
 */
namespace DoorLock::Storage::Reader {

/**
 * @brief Initializes the Reader storage module.
 *
 * Loads all persisted reader credentials into the in-memory cache.
 * Must be called once at boot before any other Reader storage operations.
 *
 * @return ALIRO_NO_ERROR on success, error code otherwise.
 */
AliroError Init();

/**
 * @brief Checks if the Reader identifier is set.
 *
 * @return True if the Reader identifier is set, false otherwise.
 */
bool IsIdentifierSet();

/**
 * @brief Gets the Reader identifier.
 *
 * @param identifier The Reader identifier.
 *
 * @return ALIRO_NO_ERROR on success, error code otherwise.
 */
AliroError GetIdentifier(Aliro::Identifier &identifier);

/**
 * @brief Sets the Reader identifier.
 *
 * Persists the identifier to PSA Protected Storage and updates the cache.
 *
 * @param identifier The Reader identifier.
 *
 * @return ALIRO_NO_ERROR on success, error code otherwise.
 */
AliroError SetIdentifier(const Aliro::Identifier &identifier);

/**
 * @brief Clears the Reader identifier.
 *
 * @return ALIRO_NO_ERROR on success, error code otherwise.
 */
AliroError ClearIdentifier();

/**
 * @brief Checks if the Reader private key is present in PSA key storage.
 *
 * @return True if the Reader private key is set, false otherwise.
 */
bool IsPrivateKeySet();

/**
 * @brief Gets the Reader public key.
 *
 * Returns the public key derived from the stored private key (cached on import).
 *
 * @param publicKey The Reader public key.
 *
 * @return ALIRO_NO_ERROR on success, error code otherwise.
 */
AliroError GetPublicKey(Aliro::CryptoTypes::PublicKey &publicKey);

/**
 * @brief Sets the Reader private key.
 *
 * Imports the private key into PSA key storage and caches the derived public key.
 *
 * @param privateKey The Reader private key.
 *
 * @return ALIRO_NO_ERROR on success, error code otherwise.
 */
AliroError SetPrivateKey(const Aliro::CryptoTypes::PrivateKey &privateKey);

/**
 * @brief Clears the Reader private key and cached public key.
 *
 * @return ALIRO_NO_ERROR on success, error code otherwise.
 */
AliroError ClearPrivateKey();

#ifdef CONFIG_DOOR_LOCK_READER_CERTIFICATE

/**
 * @brief Checks if the Reader certificate is set.
 *
 * @return True if the Reader certificate is set, false otherwise.
 */
bool IsCertificateSet();

/**
 * @brief Gets the Reader certificate.
 *
 * @param certificate The Reader certificate.
 *
 * @return ALIRO_NO_ERROR on success, error code otherwise.
 */
AliroError GetCertificate(Aliro::ConstData &certificate);

/**
 * @brief Sets the Reader certificate.
 *
 * @param certificate Pointer to the certificate data.
 * @param certificateLength Length of the certificate data in bytes.
 *
 * @return ALIRO_NO_ERROR on success, error code otherwise.
 */
AliroError SetCertificate(const uint8_t *certificate, size_t certificateLength);

/**
 * @brief Clears the Reader certificate.
 *
 * @return ALIRO_NO_ERROR on success, error code otherwise.
 */
AliroError ClearCertificate();

/**
 * @brief Checks if the Reader System Issuer CA public key is set.
 *
 * @return True if the Issuer public key is set, false otherwise.
 */
bool IsIssuerPublicKeySet();

/**
 * @brief Gets the Reader System Issuer CA public key.
 *
 * Used to verify the reader certificate chain.
 *
 * @param publicKey The Issuer public key.
 *
 * @return ALIRO_NO_ERROR on success, error code otherwise.
 */
AliroError GetIssuerPublicKey(Aliro::CryptoTypes::PublicKey &publicKey);

/**
 * @brief Sets the Reader System Issuer CA public key.
 *
 * @param publicKey The Issuer public key (must have 0x04 uncompressed prefix).
 *
 * @return ALIRO_NO_ERROR on success, error code otherwise.
 */
AliroError SetIssuerPublicKey(const Aliro::CryptoTypes::PublicKey &publicKey);

/**
 * @brief Clears the Reader System Issuer CA public key.
 *
 * @return ALIRO_NO_ERROR on success, error code otherwise.
 */
AliroError ClearIssuerPublicKey();

#endif // CONFIG_DOOR_LOCK_READER_CERTIFICATE

#ifdef CONFIG_DOOR_LOCK_BLE_UWB

/**
 * @brief Checks if the Group Resolving Key is set.
 *
 * @return True if the Group Resolving Key is set, false otherwise.
 */
bool IsGroupResolvingKeySet();

/**
 * @brief Gets the Group Resolving Key.
 *
 * Used for BLE address resolution in BLE+UWB transactions.
 *
 * @param groupResolvingKey The Group Resolving Key.
 *
 * @return ALIRO_NO_ERROR on success, error code otherwise.
 */
AliroError GetGroupResolvingKey(Aliro::CryptoTypes::GroupResolvingKey &groupResolvingKey);

/**
 * @brief Sets the Group Resolving Key.
 *
 * Imports the key into PSA key storage, replacing any existing key.
 *
 * @param groupResolvingKey The Group Resolving Key.
 *
 * @return ALIRO_NO_ERROR on success, error code otherwise.
 */
AliroError SetGroupResolvingKey(const Aliro::CryptoTypes::GroupResolvingKey &groupResolvingKey);

/**
 * @brief Clears the Group Resolving Key.
 *
 * @return ALIRO_NO_ERROR on success, error code otherwise.
 */
AliroError ClearGroupResolvingKey();

#endif // CONFIG_DOOR_LOCK_BLE_UWB

/**
 * @brief Removes all reader credentials from both persistent storage and cache.
 *
 * @return ALIRO_NO_ERROR on success, error code otherwise.
 */
AliroError ClearAll();

} // namespace DoorLock::Storage::Reader
