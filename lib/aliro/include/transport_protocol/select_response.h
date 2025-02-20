/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include "consts.h"
#include "util/span.h"
#include <optional>

namespace Aliro {

namespace TLV {
class TLVReader;
}

/**
 * @struct SELECTResponse
 *
 * Provides methods for deserialize and get data from the SELECT response.
 */
struct SELECTResponse {
	/* Maximum number of supported protocol versions expected in single SELECT response.
	 * The following value is calculated based on the table 10-2 from the Aliro Spec.
	 **/
	static constexpr size_t kMaxResponseVersionsNumber{ 47 };

	struct ProtocolVersionList {
		std::array<uint16_t, kMaxResponseVersionsNumber> version{};
		size_t versionsNum{ 0 };
	};

	using VendorExtension = SharedByteSpan;
	using ApplicationId = StaticByteSpan<kAidLen>;
	using ApplicationType = StaticByteSpan<kApplicationTypeLen>;
	using MaxCommandApdu = std::optional<uint16_t>;
	using MaxResponseApdu = std::optional<uint16_t>;
	using ProprietaryInformation = SharedByteSpan;

	/**
	 * @brief Constructor for SELECTResponse.
	 * Initializes a SELECTResponse object to read from a single buffer.
	 *
	 * @param data [input] A buffer containing the SELECT response data.
	 * @param length [input] The length of the data buffer.
	 */
	SELECTResponse(const Ptr data, size_t length);

	/**
	 * @brief Deserialize the SELECT response.
	 * The method should be called before any below get method.
	 *
	 * @return True when success, false otherwise.
	 **/
	bool Deserialize();

	/**
	 * @brief Returns the AID of a selected application.
	 *
	 * @return Application identifier.
	 */
	ApplicationId GetApplicationIdentifier() const;

	/**
	 * @brief Returns the application type Aliro application.
	 *
	 * @return Application type.
	 */
	ApplicationType GetApplicationType() const;

	/**
	 * @brief Returns the supported protocol versions for expedited phase.
	 *
	 * @return Protocol version list.
	 */
	std::optional<ProtocolVersionList> GetSupportedProtocolVersions() const;

	/**
	 * @brief Returns the maximum number of bytes in a command APDU.
	 *
	 * @return Maximum APDU command size.
	 */
	MaxCommandApdu GetMaximumCommandApdu() const;

	/**
	 * @brief Returns the maximum number of bytes in a response APDU.
	 *
	 * @return Maximum APDU response size.
	 */
	MaxCommandApdu GetMaximumResponseApdu() const;

	/**
	 * @brief Returns the vendor specific extension received in the SELECT response message.
	 *
	 * @return The vendor specific extension payload.
	 */
	std::optional<VendorExtension> GetVendorExtension() const;

	/**
	 * @brief Returns the proprietary information copied from the SELECT response message.
	 * NOTE: For a salt generation the value should contains Tag, length and value according to
	 * Table 10-2 in the Aliro Spec.
	 *
	 * @return The proprietary information in TLV format.
	 */
	ProprietaryInformation GetProprietaryInformation() const;

private:
	/**
	 * @brief Copies the proprietary information value from the SELECT response message and creates
	 * TLV structure for further processing.
	 *
	 * @return True when success, false otherwise.
	 */
	bool CopyProprietaryInfo(TLV::TLVReader &reader);

	Ptr mData{ nullptr };
	size_t mLength{ 0 };

	ApplicationId mApplicationId{};
	ApplicationType mApplicationType{};
	ProtocolVersionList mProtocolVersionList{};
	MaxCommandApdu mMaxCommandApdu{};
	MaxResponseApdu mMaxResponseApdu{};
	VendorExtension mVendorExtension{};
	ProprietaryInformation mProprietaryInformation{};
};

} // namespace Aliro
