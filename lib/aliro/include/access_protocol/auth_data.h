/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include "common.h"

namespace {

static constexpr uint8_t kUsageLength{ 4 };

} // namespace

namespace Aliro {

/**
 * @struct AUTH1AuthenticationData
 *
 * @brief Provides methods for serializing authentication data for the Reader or a User Device.
 * The serialized data are in convenient TLV format according to the Aliro Spec. Tables 8-11 and 8-12.
 */
struct AUTH1AuthenticationData {
	using Usage = StaticByteSpan<kUsageLength>;

	/**
	 * @enum DeviceAuthenticationType.
	 * @brief Represents the type of device authentication.
	 */
	enum class DeviceAuthenticationType : uint8_t {
		UserDevice = 0x00,
		Reader = 0x01,
	};

	/**
	 * @brief Constructor for AUTH1AuthenticationData sets the usage data based on the DeviceAuthenticationType.
	 * Aliro spec. 0.9.0 Tables 8-11 and 8-12: for the Reader and User Device the usage field in a TLV payload has
	 * a different value.
	 *
	 * @param device [input] a device type for which authentication data are generated.
	 */
	AUTH1AuthenticationData(DeviceAuthenticationType device);

	/**
	 * @brief Serializes data to the TLV data format.
	 *
	 * @return When success a SharedByteSpan object containing the TLV payload, empty SharedByteSpan otherwise.
	 */
	SharedByteSpan Serialize() const;

	/**
	 * @brief Sets credential ephemeral public key X coordinate.
	 *
	 * @param credentialEPubKeyX [input] Public key X coordinate.
	 */
	void SetCredentialEPubKX(const PublicKeyXcoordinate &credentialEPubKeyX);

	/**
	 * @brief Sets reader ephemeral public key X coordinate.
	 *
	 * @param readerEphemeralPubKeyX [input] Public key X coordinate.
	 */
	void SetReaderEPubKX(const PublicKeyXcoordinate &readerEphemeralPubKeyX);

	/**
	 * @brief Sets current transaction identifier.
	 *
	 * @param transactionIdentifier [input] transaction identifier for current transaction.
	 */
	void SetTransactionId(const TransactionIdentifier &transactionIdentifier);

private:
	PublicKeyXcoordinate mCredentialEPubKeyX{};
	PublicKeyXcoordinate mReaderEPubKeyX{};
	TransactionIdentifier mTransactionIdentifier{};
	Usage mUsage{};
};

} // namespace Aliro
