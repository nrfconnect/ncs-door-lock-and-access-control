/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include "common.h"

#include <optional>

namespace Aliro {

/**
 * @struct AUTH0Response
 *
 * Provides methods for deserialize and get data from the AUTH0 response.
 */
struct AUTH0Response {
	using Cryptogram = StaticByteSpan<kCryptogramLength>;
	using Ptr = Byte *;

	/**
	 * @brief Constructor for AUTH0Response.
	 * Initializes a AUTH0Response object to read from a single buffer.
	 *
	 * @param data [input] A buffer containing the AUTH0 response data.
	 * @param length [input] The length of the data buffer.
	 */
	AUTH0Response(const Ptr data, size_t length);

	/**
	 * @brief Deserialize the AUTH0 Response.
	 *
	 * @return True when success, false otherwise.
	 **/
	bool Deserialize();

	/**
	 * @brief Returns the User Device ephemeral public key received in the AUTH0 response.
	 *
	 * @return The User Device ephemeral public key.
	 */
	PublicKey GetCredentialEphemeralPubKey() const;

	/**
	 * @brief Returns the cryptogram received in the AUTH0 response message.
	 *
	 * @return The cryptogram payload.
	 */
	std::optional<Cryptogram> GetCryptogram() const;

	/**
	 * @brief Returns the vendor specific extension received in the AUTH0 response message.
	 *
	 * @return The vendor specific extension payload.
	 */
	std::optional<VendorExtension> GetVendorExtension() const;

private:
	Ptr mData{ nullptr };
	size_t mLength{ 0 };
	PublicKey mCredentialEphemeralPubKey{};
	Cryptogram mCryptogram{};
	VendorExtension mVendorExtension{};
};

} // namespace Aliro
