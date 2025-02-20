/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include "errors.h"
#include "util/span.h"
#include "util/static_byte_span.h"
#include "util/tags.h"

namespace Aliro {

namespace TLV {

/**
 * @class TLVWriter
 * @brief Provides a writer for data encoded in TLV format.
 *
 * The TLVWriter writes data directly to a SharedByteSpan buffer.
 */
class TLVWriter {
public:
	/**
	 * @brief Defult constructor for TLVWriter.
	 *
	 * Initializes a TLVWriter object.
	 */
	TLVWriter() = default;

	/**
	 * @brief Initializes a TLVWriter object to write to a single buffer.
	 *
	 * @param span [input] a buffer to write the TLV data to.
	 *
	 * @return ALIRO_NO_ERROR when success, ALIRO_INVALID_ARGUMENT otherwise.
	 */
	AliroError Init(SharedByteSpan *span);

	/**
	 * @brief Writes a TLV tag only.
	 * Aliro spec. 0.9.0 table 8-14: Exchange command, tag 0x98 has no value, length is 0.
	 *
	 * @param tag [input] the tag for the TLV element.
	 *
	 * @return ALIRO_NO_ERROR when success, error code otherwise.
	 */
	AliroError Put(const Tag &tag);

	/**
	 * @brief Writes a TLV element with the given tag and a Byte value.
	 *
	 * @param tag [input] the tag for the TLV element.
	 * @param value [input] the value for the TLV element.
	 *
	 * @return ALIRO_NO_ERROR when success, error code otherwise.
	 */
	AliroError Put(const Tag &tag, Byte value);

	/**
	 * @brief Writes a TLV element with the given tag and 16-bit unsigned integer value.
	 *
	 * @param tag [input] the tag for the TLV element.
	 * @param value [input] the value for the TLV element.
	 *
	 * @return ALIRO_NO_ERROR when success, error code otherwise.
	 */
	AliroError Put(const Tag &tag, uint16_t value);

	/**
	 * @brief Writes a TLV element with the given tag and StaticByteSpan or SharedByteSpan value.
	 *
	 * @param tag [input] the tag for the TLV element.
	 * @param data [input] the value for the TLV element.
	 *
	 * @return ALIRO_NO_ERROR when success, error code otherwise.
	 */
	template <typename T> AliroError Put(const Tag &tag, const T &data)
	{
		// Tag and Length
		static constexpr size_t kTlvHeaderLength{ 2 };
		VerifyOrReturnStatus(mInitialized, ALIRO_INVALID_STATE);

		StaticByteSpan<kTlvHeaderLength> tlvHeader{};
		tlvHeader.Append(ToUnderlying(tag));
		tlvHeader.Append(data.Size());

		*mSpan += tlvHeader;
		*mSpan += data;

		VerifyOrReturnStatus(mSpan, ALIRO_NO_MEMORY);

		return ALIRO_NO_ERROR;
	}

private:
	SharedByteSpan *mSpan{ nullptr };
	bool mInitialized{ false };
};

} // namespace TLV

} // namespace Aliro
