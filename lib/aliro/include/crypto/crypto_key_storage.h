/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include "crypto_common.h"

namespace Aliro {

/**
 * @class EccPublicKey
 * @brief A class for public key representation.
 */
class EccPublicKey {
public:
	/**
	 * @brief Returns a X coordinate for related EC public key.
	 *
	 * @return The X coordinate for EC public key.
	 */
	const PublicKeyXcoordinate &GetXCoordinate() const;

	/**
	 * @brief Returns a public key ID
	 *
	 * @return Public key ID.
	 */
	KeyId GetPublicKey() const;

	/**
	 * @brief Sets a public key in a secure storage key slot.
	 * The function additionally stores the public key X coordinate in member variable for future use.
	 *
	 * @param publicKey input public key.
	 *
	 * @return ALIRO_NO_ERROR when the key was stored with successfully, an error code otherwise.
	 */
	AliroError Set(const EccP256PublicKey &publicKey);

	/**
	 * @brief Releases the secure storage key slot and sets zeros for X coordinate.
	 *
	 * @return ALIRO_NO_ERROR when the key slot is released with successful, an error code otherwise.
	 */
	AliroError Free();

private:
	PublicKeyXcoordinate mKeyCoordinateX{};
	KeyId mKeyId{};
};

/**
 * @class CryptoKeyStorage
 * @brief Simple helper class for reader's long term keys (LTK) storage.
 */
class CryptoKeyStorage {
public:
	static CryptoKeyStorage &Instance()
	{
		static CryptoKeyStorage sCryptoKeyStorage;
		return sCryptoKeyStorage;
	}

	/**
	 * @brief Key storage initialization function.
	 *
	 * Initializes a key storage backend and loads a reader's LTK into it.
	 *
	 * @return ALIRO_NO_ERROR when success, error code otherwise.
	 */
	AliroError Init();

	/**
	 * @brief Returns a X coordinate extracted from reader's LTK public key.
	 *
	 * @return The public key X coordinate.
	 */
	const StaticByteSpan<kEccP256PublicKeyCoordinateXLength> &GetReaderPublicKeyXCoordinate() const
	{
		return mPublicKey.GetXCoordinate();
	}

	/**
	 * @brief Returns private key ID slot where LTK private key is stored.
	 *
	 * @return The private key ID.
	 */
	uint32_t GetReaderPrivateKeyId() const;

	/**
	 * @brief Sets a public key in a secure storage key slot.
	 * The function additionally stores the public key X coordinate in member variable for future use.
	 *
	 * @param accessCredentialPublicKey input public key.
	 *
	 * @return ALIRO_NO_ERROR when the key was stored with successfully, an error code otherwise.
	 */
	AliroError SetAccessCredentialPublicKey(const PublicKey &accessCredentialPublicKey);

	/**
	 * @brief Returns public key ID slot where LTK public key is stored.
	 *
	 * @return The public key ID.
	 */
	uint32_t GetAccessCredentialPublicKeyId() const;

	/**
	 * @brief Returns a X coordinate extracted from reader's LTK public key.
	 *
	 * @return The public key X coordinate.
	 */
	const StaticByteSpan<kEccP256PublicKeyCoordinateXLength> &GetAccessCredentialPublicKeyXCoordinate() const
	{
		return mAccessCredentialPublicKey.GetXCoordinate();
	}

private:
	/**
	 * @brief Default constructor for CryptoKeyStorage initializes 'empty' kyes storage.
	 */
	CryptoKeyStorage() = default;

	/**
	 * @brief The key storage should not be cloneable.
	 */
	CryptoKeyStorage(CryptoKeyStorage &other) = delete;

	/**
	 * @brief The key storage should not be assignable.
	 */
	void operator=(const CryptoKeyStorage &) = delete;

	/**
	 * @brief The key storage should not be movable.
	 */
	CryptoKeyStorage(CryptoKeyStorage &&) = delete;
	CryptoKeyStorage &operator=(CryptoKeyStorage &&) = delete;

	/**
	 * @brief Default destructor.
	 */
	~CryptoKeyStorage() = default;

	uint32_t mPrivateKeyId{};
	EccPublicKey mPublicKey{};
	EccPublicKey mAccessCredentialPublicKey{};
};

} // namespace Aliro
