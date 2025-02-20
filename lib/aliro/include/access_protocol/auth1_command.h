/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include "access_protocol/common.h"

namespace Aliro {

/**
 * @struct AUTH1Command
 *
 * @brief Provides methods for serializing AUTH1 command to the TLV format according to the Aliro Spec. Table 8-9.
 */
struct AUTH1Command {
	/**
	 * @enum AccessCredentialType.
	 * @brief Represents the Access Credential key type request.
	 */
	enum class AccessCredentialType : uint8_t { KeySlot = 0x00, AccessCredentialPublicKey = 0x01 };

	/**
	 * @brief Default constructor for AUTH1Command.
	 *
	 * Initializes a AUTH1Command object with default data.
	 */
	AUTH1Command() = default;

	/**
	 * @brief Serializes data to the TLV data format.
	 *
	 * @return When success a SharedByteSpan object containing the TLV payload, empty SharedByteSpan otherwise.
	 */
	SharedByteSpan Serialize() const;

	/**
	 * @brief Sets access credential type for command parameters field.
	 * Currently command parameters contains ony one bit, all other bits are RFU.
	 *
	 * @param accessCredential [input] the access credential type.
	 */
	void SetCommandParameters(AccessCredentialType accessCredential);

	/**
	 * @brief Sets the reader's signature.
	 *
	 * @param signature [input] the reader signature.
	 */
	void SetSignature(const Signature &signature);

private:
	CommandParameters mCommandParameters{ 0x00 };
	Signature mSignature{};
};

}; // namespace Aliro
