/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include "crypto/crypto_common.h"
#include "util/span.h"

namespace Aliro {

/**
 * @brief A class that represents a salt used for cryptographic operations.
 * NOTE: The salt value is not considered as a sensitive data, however it is used
 * for cryptographic operations.
 */
class Salt {
	static constexpr size_t kFlagLength = kCommandParametersLength + kAuthenticationPolicyLength;

public:
	using ProtocolVersion = StaticByteSpan<kProtocolVersionLength>;

	enum class SaltType : uint8_t { VolatileFast = 0, Volatile, Persistent };

	Salt(InterfaceByte transportInterface = InterfaceByte::NFC)
	{
		mTransportInterface.Append(ToUnderlying(transportInterface));
	}

	/**
	 * @brief Set proprietary info.
	 * The method MUST be called before the `Create` method will be invoked.
	 *
	 * @param proprietaryInfo input proprietary info.
	 */
	void SetProprietaryInfo(const SharedByteSpan &proprietaryInfo);

	/**
	 * @brief Set parameters required for salt's `flag`.
	 * The method MUST be called before the `Create` method will be invoked.
	 *
	 * @param commandParameters input command parameters used in AUTH0 command.
	 * @param authenticationPolicy input authentication policy used in AUTH0 command.
	 */
	void SetFlag(Byte commandParameters, Byte authenticationPolicy);

	/**
	 * @brief Set Access Credential public key X coordinate.
	 * The method MUST be called before the `Create` method will be invoked.
	 *
	 * @param acessCredentialKeyX input public key X coordinate.
	 */
	void SetAccessCredentialPubKeyX(const PublicKeyXcoordinate &acessCredentialKeyX);

	/**
	 * @brief Set current protocol version.
	 * The method MUST be called before the `Create` method will be invoked.
	 *
	 * @param protocolVersion input the current protocol version.
	 */
	void SetProtocolVersion(const ProtocolVersion &protocolVersion);

	/**
	 * @brief Set current transaction identifier.
	 * The method MUST be called before the `Create` method will be invoked.
	 *
	 * @param transactionIdentifier input transaction identifier for current transaction.
	 */
	void SetTransactionId(const TransactionIdentifier &transactionIdentifier);

	/**
	 * @brief Create a salt for current transaction.
	 * The salt type depends on access protocol phase.
	 * NOTE: Call this function for each new transaction or when new salt must be created.
	 *
	 * @param type input salt type to be created.
	 *
	 * @return ALIRO_NO_ERROR when success, ALIRO_INVALID_ARGUMENT when `type` is invalid or
	 * any of the material need to generate salt is missing.
	 */
	AliroError CreateSalt(SaltType type = SaltType::Volatile);

	/**
	 * @brief Get created salt.
	 * The `Create` method must be invoked inorder to create proper salt.
	 *
	 * @return salt when created successfully, an empty SharedByteSpan otherwise.
	 */
	const SharedByteSpan &GetSalt() const;

	/**
	 * @brief Explicitly destroy salt and release allocated memory.
	 */
	void ClearSalt();

private:
	SharedByteSpan mData{};
	SharedByteSpan mProprietaryInfo{};
	PublicKeyXcoordinate mAccessCredentialX{};
	TransactionIdentifier mTransactionIdentifier{};
	StaticByteSpan<kFlagLength> mFlag{};
	StaticByteSpan<sizeof(Byte)> mTransportInterface{};
	ProtocolVersion mProtocolVersion{};
};

} // namespace Aliro
