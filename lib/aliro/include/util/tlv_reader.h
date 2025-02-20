/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
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
 * @class TLVReader
 * @brief Provides a parser for data encoded in TLV format.
 *
 * A TLVReader object parses data directly from a fixed input buffer.
 * @note If the provided buffer has malformed data (e.g. data are not in TLV format) results returned by parser are
 * undefined.
 */
class TLVReader {
public:
	/**
	 * @brief Defult constructor for TLVReader.
	 *
	 * Initializes a TLVReader object as uninitialized with `Tag::Invalid` Tag.
	 */
	TLVReader() = default;

	/**
	 * @brief Initializes a TLVReader object to read from a single buffer.
	 *
	 * @note If the TLVReader object is not initialized the GetTag() method returns Tag::Invalid and the GetLength()
	 * method returns 0.
	 *
	 * @param rawData [input] a buffer containg a raw data to be parsed.
	 * @param dataLen [input] the length of the data in the rawData buffer.
	 *
	 * @return ALIRO_NO_ERROR when success, ALIRO_INVALID_ARGUMENT otherwise.
	 */
	AliroError Init(const Byte *rawData, size_t dataLen);

	/**
	 * @brief Checks if the TLVReader object is already initialized.
	 *
	 * @return true when object is initialized, false otherwise.
	 */
	bool Initialized() const;

	/**
	 * @brief Returns the tag associated with current TLV element.
	 *
	 * @return The tag associated wit the current TLV element, Tag::Invalid otherwise.
	 */
	Tag GetTag() const;

	/**
	 * @brief Returns the length in bytes of the value field associated with the current TLV element.
	 *
	 * @return The length of value associated with the current TLV element, or 0 when the TLVReader object is not
	 * initialized.
	 */
	size_t GetLength() const;

	/**
	 * @brief Positions the TLV reader on the next TLV element in the same containment context.
	 *
	 * @note Calling Next() will advance the TLV reader pointer to the next concatenated tag and skip nested TLV
	 * elements. In order to get nested tag(s) use one of NextNested methods. When there is no further elements in
	 * the same containment context the ALIRO_TLV_END_OF_TLV error code will be returned, and associated tag will
	 * stay unchanged.
	 *
	 * @return ALIRO_NO_ERROR when the TLV reader is positioned on the next TLV element, the ALIRO_TLV_END_OF_TLV
	 * error code when there are no further element in the same containment context, or other error code orherwise.
	 */
	AliroError Next();

	/**
	 * @brief Positions the TLV reader on the next TLV element in the same containment context and validates
	 * if the reader is positioned at an elemement with the expected tag.
	 *
	 * @note Calling Next() will advance the TLV reader pointer to the next concatenated tag and skip nested TLV
	 * elements. In order to get nested tag(s) use one of NextNested methods. When there is no further elements in
	 * the same containment context the ALIRO_TLV_END_OF_TLV error code will be returned, and associated tag will
	 * stay unchanged.
	 *
	 * @param expectedTag [input] expected tag.
	 *
	 * @return ALIRO_NO_ERROR when the TLV reader is positioned on the next expected TLV element, the
	 * ALIRO_TLV_END_OF_TLV error code when there are no further element in the same containment context, or other
	 * error code orherwise.
	 */
	AliroError Next(Tag expectedTag);

	/**
	 * @brief Positions the TLV reader on the next TLV element in the same containment context and validates
	 * if the reader is positioned at an elemement with the expected tag and length.
	 *
	 * @note Calling Next() will advance the TLV reader pointer to the next concatenated tag and skip nested TLV
	 * elements. In order to get nested tag(s) use one of NextNested methods. When there is no further elements in
	 * the same containment context the ALIRO_TLV_END_OF_TLV error code will be returned, and associated tag will
	 * stay unchanged.
	 *
	 * @param expectedTag [input] expected tag.
	 * @param expectedLen [input] expected length for the associated tag.
	 *
	 * @return ALIRO_NO_ERROR when the TLV reader is positioned on the next TLV element and expected tag and length
	 * are matched, the ALIRO_TLV_END_OF_TLV error code when there are no further element in the same containment
	 * context, or other error code orherwise.
	 */
	AliroError Next(Tag expectedTag, size_t expectedLen);

	/**
	 * @brief Verify wheter the TLV reader is positioned at an elemement with the expected tag.
	 *
	 * @param expectedTag [input] expected tag.
	 *
	 * @return ALIRO_NO_ERROR when the expected tag is present, error code otherwise.
	 */
	AliroError Expect(Tag expectedTag) const;

	/**
	 * @brief Validates wheter the TLV reader is positioned at an elemement with the expected tag and length
	 * associated with the tag is the same as expected.
	 *
	 * @param expectedTag [input] expected tag.
	 * @param expectedLen [input] expected length for the associated tag.
	 *
	 * @return ALIRO_NO_ERROR when the expected tag and length are matched, error code otherwise.
	 */
	AliroError Expect(Tag expectedTag, size_t expectedLen) const;

	/**
	 * @brief Get the value associated with current TLV element as a Byte.
	 *
	 * @param value [output] receives the value associated with current TLV element.
	 *
	 * @return ALIRO_NO_ERROR when success, error code otherwise.
	 */
	AliroError GetValue(Byte &value) const;

	/**
	 * @brief Get the value associated with current TLV element as a 16-bit unsigned integer.
	 *
	 * @param value [output] receives the value associated with current TLV element.
	 *
	 * @return ALIRO_NO_ERROR when success, error code otherwise.
	 */
	AliroError GetValue(uint16_t &value) const;

	/**
	 * @brief Get the value associated with current TLV element as a StaticByteSpan.
	 *
	 * @param data [output] receives the value associated with current TLV element.
	 *
	 * @return ALIRO_NO_ERROR when success, error code otherwise.
	 */
	template <Capacity C> AliroError GetValue(StaticByteSpan<C> &data) const
	{
		VerifyOrReturnStatus(mInitialized, ALIRO_INVALID_STATE);
		VerifyOrReturnStatus(data.Capacity() >= mCurrentLength, ALIRO_TLV_BUFFER_TOO_SMALL);

		return data.Set(mCurrentDataPosition, mCurrentLength);
	}

	/**
	 * @brief Get the value associated with current TLV element as a SharedByteSpan.
	 *
	 * @param data [output] receives the value associated with current TLV element.
	 *
	 * @return ALIRO_NO_ERROR when success, error code otherwise.
	 */
	AliroError GetValue(SharedByteSpan &data) const;

	/**
	 * @brief Advances the TLV reader to nested TLV element to be read in the same containment context.
	 *
	 * @note Calling NextNested() will advance the TLV reader pointer to the nested TLV element and updates
	 * mDataPosition and mDataEnd. Call this function when any next concatenated TLV element is not expected or
	 * backup TLVReader object with containment context.
	 *
	 * @return ALIRO_NO_ERROR when success, error code otherwise.
	 */
	AliroError NextNested();

	/**
	 * @brief Advances the TLV reader to nested TLV element to be read in the same containment context and validates
	 * if the TLV reader is positioned at an elemement with the expected tag.
	 *
	 * @note Calling NextNested() will advance the TLV reader pointer to the nested TLV element and updates
	 * mDataPosition and mDataEnd. Call this function when any next concatenated TLV element is not expected or
	 * backup TLVReader object with containment context.
	 *
	 * @param expectedTag [input] expected tag.
	 *
	 * @return ALIRO_NO_ERROR when the expected tag is present, error code otherwise.
	 */
	AliroError NextNested(Tag expectedTag);

	/**
	 * @brief Advances the TLV reader to nested TLV element to be read in the same containment context aand
	 * validates if the TLV reader is positioned at an elemement with the expected tag and length.
	 *
	 * @note Calling NextNested() will advance the TLV reader pointer to the nested TLV element and updates
	 * mDataPosition and mDataEnd. Call this function when any next concatenated TLV element is not expected or
	 * backup TLVReader object with containment context.
	 *
	 * @param expectedTag [input] expected tag.
	 * @param expectedLen [input] expected length for the associated tag.
	 *
	 * @return ALIRO_NO_ERROR when the TLV reader is positioned on the next TLV element and expected tag and length
	 * are matched, the ALIRO_TLV_END_OF_TLV error code when there are no further element in the same containment
	 * context, or other error code orherwise.
	 */
	AliroError NextNested(Tag expectedTag, size_t expectedLen);

private:
	Tag mCurrentTag{ Tag::Invalid };
	size_t mCurrentLength{ 0 };
	const Byte *mCurrentDataPosition{ nullptr };
	const Byte *mDataEnd{ nullptr };
	bool mInitialized{ false };
};

} // namespace TLV

} // namespace Aliro
