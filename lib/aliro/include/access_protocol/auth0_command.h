/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include "common.h"
#include "protocol_versions/protocol_version_arbiter.h"

namespace Aliro {

/**
 * @struct AUTH0Command
 *
 * @brief Provides methods for serializing AUTH0 command to the TLV format according to the Aliro Spec. Table 8-3.
 */
struct AUTH0Command {
	/**
	 * @enum ExpeditedPhaseType.
	 * @brief Represents the request expedited phase type.
	 */
	enum class ExpeditedPhaseType : uint8_t { ExpeditedStandard = 0x00, ExpeditedFast = 0x01 };

	/**
	 * @enum UserAuthenticationPolicy.
	 * @brief Represents the user authentication policy.
	 */
	enum class UserAuthenticationPolicy : uint8_t {
		Unknown = 0x00,
		UserDeviceSettings = 0x01,
		UserDeviceSettingsSecureAction = 0x02,
		// Reader should not use “Force user authentication” option when performing BLE+UWB
		ForceUserAuth = 0x03,
		// RFU is defined as a range -> 0x00, 0x04-0xFF
	};

	/**
	 * @brief Default constructor for AUTH0Command.
	 *
	 * Initializes a AUTH0Command object with default data.
	 */
	AUTH0Command() = default;

	/**
	 * @brief Serializes data to the TLV data format.
	 *
	 * @return When success a SharedByteSpan object containing the TLV payload, empty SharedByteSpan otherwise.
	 */
	SharedByteSpan Serialize() const;

	/**
	 * @brief Sets requested expedited phase type for command parameters field.
	 * Currently command parameters contains ony one bit, all other bits are RFU.
	 *
	 * @param expeditedPhase [input] the requested expedited phase type.
	 */
	void SetCommandParameters(ExpeditedPhaseType expeditedPhase);

	/**
	 * @brief Sets user authentication policy field based on UserAuthenticationPolicy.
	 *
	 * @param authPolicy [input] the selected authentication policy.
	 */
	void SetUserAuthenticationPolicy(const UserAuthenticationPolicy &authPolicy);

	/**
	 * @brief Sets current protocol verion.
	 *
	 * @param protocolVersion [input] the protocol version.
	 */
	void SetProtocolVersion(const ProtocolVersionArbiter::ProtocolVersionType &protocolVersion);

	/**
	 * @brief Sets ephemeral public key generated for current session.
	 *
	 * @param readerEphemeralKey [input] the public key.
	 */
	void SetEphemeralPubKey(const PublicKey &readerEphemeralKey);

	/**
	 * @brief Sets unique transaction ID generated for current session.
	 *
	 * @param transacionId [input] the transaction identifier.
	 */
	void SetTransactionIdentifier(const TransactionIdentifier &transacionId);

private:
	CommandParameters mCommandParameters{ 0x00 };
	UserAuthenticationPolicy mUserAuthPolicy{ UserAuthenticationPolicy::Unknown };
	ProtocolVersionArbiter::ProtocolVersionType mProtocolVersion{};
	PublicKey mReaderEPubKey{};
	TransactionIdentifier mTransactionIdentifier{};
};

} // namespace Aliro
