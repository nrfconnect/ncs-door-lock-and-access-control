/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include "access_protocol/common.h"
#include "consts.h"
#include "util/static_byte_span.h"
#include "util/tags.h"

#include <zephyr/sys/util.h>

#include <optional>

namespace Aliro {

namespace TLV {
class TLVReader;
}

class SignalingBitmap {
public:
	using Data = uint16_t;

	// spec Table 8-10
	enum class Bits : uint16_t {
		AccessDocAvailable = BIT(0),
		RevocationDocAvailable = BIT(1),
		StepUpAIDRequired = BIT(2), // only applicable for NFC transport
		MailboxDataAvailable = BIT(3),
		MailboxDataReadable = BIT(4),
		MailboxDataWritable = BIT(5),
		CredentialBackendSupported = BIT(6),
		SendingDataToAppSupported = BIT(7),
		Reserved1 = BIT(8),
		UpdateDocSupportedInExpedited = BIT(9),
		MailboxSupportedInExchangeCmd = BIT(10),
		NotifySupportedInExchangeCmd = BIT(11),
		UpdateDocSupportedInStepUp = BIT(12),
		Reserved2 = BIT(13),
		Reserved3 = BIT(14),
		Reserved4 = BIT(15)
	};

	/**
	 * @brief Parse the TLV elment associated with the Tag::SignalingBitmap tag.
	 *
	 * @return True when success, false otherwise.
	 **/
	bool Deserialize(TLV::TLVReader &reader);

	/**
	 * @brief Returns the signaling bitmap as a 16-bit unsigned integer.
	 *
	 * @return The signaling bitmap.
	 **/
	Data Get() const;

	/**
	 * @brief Checks if the specific bit is set.
	 *
	 * @return True when bit is set, false otherwise.
	 **/
	bool IsBitSet(Bits bit) const;

private:
	Data mData{};
	static constexpr Tag kTag{ Tag::SignalingBitmap };
};

/**
 * @struct AUTH1Response
 *
 * Provides methods for deserialize and get data from the AUTH1 response.
 */
struct AUTH1Response {
	using KeySlot = StaticByteSpan<kKeySlotLength>;
	using SignedTimestamp = StaticByteSpan<kSignedTimestampLength>;

	/**
	 * @brief Constructor for AUTH1Response.
	 * Initializes a AUTH1Response object to read from a single buffer.
	 *
	 * @param data [input] A buffer containing the AUTH1 response data.
	 * @param length [input] The length of the data buffer.
	 */
	AUTH1Response(const Ptr data, size_t length);

	/**
	 * @brief Deserialize the AUTH1 Response.
	 *
	 * @return True when success, false otherwise.
	 **/
	bool Deserialize();

	/**
	 * @brief Returns the access credentail key slot identifier.
	 *
	 * @return The key slot identifier.
	 **/
	std::optional<KeySlot> GetKeySlot() const;

	/**
	 * @brief Returns the access credentail public key.
	 *
	 * @return The access credentail public key.
	 **/
	std::optional<PublicKey> GetLongTermPublicKey() const;

	/**
	 * @brief Returns the User Device signature to verify.
	 *
	 * @return The User Device signature.
	 **/
	Signature GetUserDeviceSignature() const;

	/**
	 * @brief Returns the mailbox data subset.
	 *
	 * @return The mailbox data subset.
	 **/
	std::optional<SharedByteSpan> GetPrivateMailboxDataSubset() const;

	/**
	 * @brief Returns the signaling bitmap as a 16-bit unsigned integer.
	 *
	 * @return The signaling bitmap.
	 **/
	SignalingBitmap::Data GetSignalingBitmapData() const;

	/**
	 * @brief Returns the signaling bitmap object.
	 *
	 * @return The signaling bitmap.
	 **/
	SignalingBitmap GetSignalingBitmap() const;

	/**
	 * @brief Returns the credential signed timestamp.
	 *
	 * @return The credential signed timestamp.
	 **/
	std::optional<SignedTimestamp> GetCredentialSignedTimestamp() const;

	/**
	 * @brief Returns the revocation signed timestamp.
	 *
	 * @return The revocation signed timestamp.
	 **/
	std::optional<SignedTimestamp> GetRevocationSignedTimestamp() const;

private:
	Ptr mData{ nullptr };
	size_t mLength{ 0 };
	KeySlot mKeySlot{};
	PublicKey mLongTermPublicKey{};
	Signature mUserDeviceSignature{};
	SharedByteSpan mPrivateMailboxDataSubset{};
	SignalingBitmap mSignalingBitmap{};
	SignedTimestamp mCredentialSignedTimestamp{};
	SignedTimestamp mRevocationSignedTimestamp{};
};

} // namespace Aliro
