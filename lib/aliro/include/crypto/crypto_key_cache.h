/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include "crypto_key_storage.h"

namespace Aliro {

struct ReaderEphemeralKey : public EccPublicKey {
	/**
	 * @brief Generates ephemeral EC key pair (private/public).
	 * The function additionally stores public key X coordinate and private key slot ID.
	 *
	 * @param readerEphemeralPublicKey [output] reader ephemeral public key.
	 *
	 * @return ALIRO_NO_ERROR when success, error code otherwise.
	 */
	AliroError Generate(PublicKey &readerEphemeralPublicKey);

	/**
	 * @brief Returns a private key ID
	 *
	 * @return Private key ID.
	 */
	KeyId GetPrivateKey() const;

	/**
	 * @brief Releases the secure storage key slots and sets zeros for public key X coordinate.
	 *
	 * @return ALIRO_NO_ERROR when the key slot is released with successful, an error code otherwise.
	 */
	AliroError Free();

private:
	KeyId mPrivateKeyId{};
};

/**
 * Helper structure for representing symmetric session-bounds key IDs (respective):
 * - ExpeditedSKReader - used for reader's messages encryption, when secure channel is established.
 * - ExpeditedSKDevice - used for user device messages decryption, when secure channel is established.
 * - StepUpSK - used for derive StepUpSKDevice and StepUpSKReader keys to protect Step-up phase.
 * - BleSK - used to protect a UWB session setup commands.
 * - URSK - as a UWB ranging secret key.
 */
struct SessionBoundKeys {
	uint32_t mExpeditedSkReader{};
	uint32_t mExpeditedSkDevice{};
	uint32_t mStepUpSk{};
	uint32_t mBleSk{};
	uint32_t mURSk{};
};

class CryptoKeyCache {
public:
	using SharedKeyId = uint32_t;

	static CryptoKeyCache &Instance()
	{
		static CryptoKeyCache sCryptoKeyCache;
		return sCryptoKeyCache;
	}

	/**
	 * @brief Returns ExpeditedSKReader key ID.
	 *
	 * @return The ExpeditedSKReader key ID.
	 */
	uint32_t GetExpeditedSkReaderKeyId() const;

	/**
	 * @brief Returns ExpeditedSKDevice key ID.
	 *
	 * @return The ExpeditedSKDevice key ID.
	 */
	uint32_t GetExpeditedSkDeviceKeyId() const;

	/**
	 * @brief Returns StepUpSK key ID.
	 *
	 * @return The StepUpSK key ID.
	 */
	uint32_t GetStepUpSkKeyId() const;

	/**
	 * @brief Returns BleSK key ID.
	 *
	 * @return The BleSK key ID.
	 */
	uint32_t GetBleSkKeyId() const;

	/**
	 * @brief Returns URSK key ID.
	 *
	 * @return The URSK key ID.
	 */
	uint32_t GetURSkKeyId() const;

	/**
	 * @brief Sets keys derived for session.
	 *
	 * @pram sessionKeys input bunch od fession keys.
	 */
	void SetSessionBoundKeys(const SessionBoundKeys &sessionKeys);

	/**
	 * @brief Stores a User Device public key.
	 *
	 * @return ALIRO_NO_ERROR when the key stored with success, an error code otherwise.
	 */
	AliroError SetUserDeviceEphemeralKey(const PublicKey &userDevicePublicKey);

	/**
	 * @brief Stores a shared key.
	 * NOTE: The key must be stored here in order to release it when a session is finished.
	 *
	 * @param sharedKey input the sared key ID.
	 */
	void SetSharedKey(const SharedKeyId &sharedKey);

	/**
	 * @brief Returns the shared key ID.
	 *
	 * @return The shared key ID.
	 */
	SharedKeyId GetSharedKey() const;

	/**
	 * @brief Returns User Device public key X coordinate.
	 *
	 * @return Ephemeral public key X coordinate when success, empty SharedByteSpan otherwise.
	 */
	PublicKeyXcoordinate GetUserDeviceEphemeralKeyXCoordinate() const;

	/**
	 * @brief Flushes all session related keys.
	 *
	 * @return ALIRO_NO_ERROR when success, an error code otherwise.
	 */
	AliroError Flush();

	/**
	 * FIXME: Temporary placed here.
	 */
	ReaderEphemeralKey mReaderEphemeralKey{};

private:
	/**
	 * @brief Default constructor for CryptoKeyCache initializes 'empty' kyes cache.
	 */
	CryptoKeyCache() = default;

	/**
	 * @brief The key cache should not be cloneable.
	 */
	CryptoKeyCache(CryptoKeyCache &other) = delete;

	/**
	 * @brief The key cache should not be assignable.
	 */
	void operator=(const CryptoKeyCache &) = delete;

	/**
	 * @brief The key cache should not be movable.
	 */
	CryptoKeyCache(CryptoKeyCache &&) = delete;
	CryptoKeyCache &operator=(CryptoKeyCache &&) = delete;

	/**
	 * @brief Default destructor.
	 */
	~CryptoKeyCache() = default;

	SessionBoundKeys mSessionBoundKeys{};
	EccPublicKey mCredentialEphemeralPubKey{};
	SharedKeyId mSharedKey{};
};

} // namespace Aliro
