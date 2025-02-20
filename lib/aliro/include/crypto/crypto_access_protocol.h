/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include "crypto/salt.h"
#include "crypto_common.h"
#include "crypto_key_storage.h"

namespace Aliro {

class AccessProtocolCrypto {
public:
	static AccessProtocolCrypto &Instance()
	{
		static AccessProtocolCrypto sAccessProtocolCrypto;
		return sAccessProtocolCrypto;
	}

	/**
	 * @brief Starts a new communication session.
	 * This method generates session related data like a EC key pairs and transaction ID.
	 *
	 * @param publicKey Output the ephemeral EC public key for session.
	 *
	 * @return ALIRO_NO_ERROR when success, an error code otherwise.
	 */
	AliroError StartSession(PublicKey &publicKey);

	/**
	 * @brief Finishes current session and destroy all session-bound keys and data.
	 *
	 * @return ALIRO_NO_ERROR when success, an error code otherwise.
	 */
	AliroError FinishSession();

	/**
	 * @brief Returns the transaction identifier.
	 * NOTE: `StartSession` should be invoked when each new session starts.
	 *
	 * @return The transaction identifier for the session.
	 */
	TransactionIdentifier GetTransactionIdentifier() const;

	/**
	 * @brief Establishes a secure channel for current session.
	 *
	 * @param userDevicePublicKey Input a User Device ephemeral EC public key.
	 *
	 * @return ALIRO_NO_ERROR when the secure channel is established, an error code otherwise.
	 */
	AliroError EstablishSecureChannel(const PublicKey &userDevicePublicKey);

	/**
	 * @brief Validates whether a secure channel is established.
	 *
	 * @return TRUE if the secure channel is established for current session, FALSE otherwise.
	 */
	bool IsEstablishedSecureChannel() const;

	/**
	 * FIXME: Temporary placed here.
	 */
	Salt mSalt{};

private:
	/**
	 * @brief Default constructor for AccessProtocolCrypto.
	 */
	AccessProtocolCrypto() = default;

	/**
	 * @brief The AccessProtocolCrypto object should not be cloneable.
	 */
	AccessProtocolCrypto(AccessProtocolCrypto &other) = delete;

	/**
	 * @brief The AccessProtocolCrypto object should not be assignable.
	 */
	void operator=(const AccessProtocolCrypto &) = delete;

	/**
	 * @brief The AccessProtocolCrypto object should not be movable.
	 */
	AccessProtocolCrypto(AccessProtocolCrypto &&) = delete;
	AccessProtocolCrypto &operator=(AccessProtocolCrypto &&) = delete;

	/**
	 * @brief Default destructor.
	 */
	~AccessProtocolCrypto() = default;

	bool mSessionStarted{ false };
	bool mSecureChannel{ false };
	TransactionIdentifier mTransactionIdentifier{};
};

} // namespace Aliro
