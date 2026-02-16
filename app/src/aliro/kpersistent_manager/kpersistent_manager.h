/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

#pragma once

#include "aliro/errors.h"
#include "aliro/types.h"

namespace Aliro {

class KpersistentManager {
public:
	KpersistentManager() = default;
	virtual ~KpersistentManager() = default;

	/**
	 * @brief Get the number of Kpersistent keys.
	 *
	 * This method must be used before calling GetKpersistentKeyIds to know
	 * the size of the preallocated buffer for the key IDs in the caller's context.
	 *
	 * @param count The number of Kpersistent keys.
	 *
	 * @return ALIRO_NO_ERROR when success, an error code otherwise.
	 */
	virtual AliroError GetKpersistentCount(size_t &count) = 0;

	/**
	 * @brief Get the IDs of the Kpersistent keys.
	 *
	 * @param keyIds The IDs of the Kpersistent keys.
	 * @param count The number of Kpersistent keys.
	 *
	 * @return ALIRO_NO_ERROR when success, an error code otherwise.
	 */
	virtual AliroError GetKpersistentKeyIds(CryptoTypes::KeyId *keyIds, size_t &count) = 0;

	/**
	 * @brief Preserve a Kpersistent key.
	 *
	 * @param publicKey The user's public key corresponding to the Kpersistent key.
	 * @param kpersistentKeyId The volatile Kpersistent key ID.
	 *
	 * @return ALIRO_NO_ERROR when success, ALIRO_KEY_ALREADY_EXISTS if the Kpersistent already exists, other error
	 * status otherwise.
	 */
	virtual AliroError PreserveKpersistent(const CryptoTypes::PublicKey &publicKey,
					       CryptoTypes::KeyId kpersistentKeyId) = 0;

	/**
	 * @brief Remove a Kpersistent key.
	 *
	 * @param kpersistentKeyOffset The offset of the Kpersistent key to be removed.
	 *
	 * @return ALIRO_NO_ERROR when success, an error code otherwise.
	 */
	virtual AliroError RemoveKpersistent(size_t kpersistentKeyOffset) = 0;

	/**
	 * @brief Remove all Kpersistent keys.
	 */
	virtual void RemoveAllKpersistent() = 0;

	/**
	 * @brief Get the ID of the Access Credential public key corresponding to the given Kpersistent key.
	 *
	 * @param kpersistentKeyId The ID of the Kpersistent key.
	 * @param publicKey The Access Credential public key.
	 *
	 * @return ALIRO_NO_ERROR when success, an error code otherwise.
	 */
	virtual AliroError GetAccessCredentialPublicKey(CryptoTypes::KeyId kpersistentKeyId,
							CryptoTypes::PublicKey &publicKey) = 0;

	KpersistentManager(const KpersistentManager &) = delete;
	KpersistentManager(KpersistentManager &&) = delete;
	KpersistentManager &operator=(const KpersistentManager &) = delete;
	KpersistentManager &operator=(KpersistentManager &&) = delete;
};

} // namespace Aliro
