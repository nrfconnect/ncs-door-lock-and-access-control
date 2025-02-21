/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include "consts.h"
#include "util/static_byte_span.h"

namespace Aliro {

/**
 * @class ReaderIdentifier
 * @brief Temporary solution for hardcoded reader group identifier.
 */
struct ReaderIdentifier {
	using Identifier = StaticByteSpan<kReaderIdentifierLength>;
	using GroupIdentifier = StaticByteSpan<kReaderGroupIdentifierLength>;
	using SubIdentifier = StaticByteSpan<kReaderGroupSubIdentifierLength>;

	static ReaderIdentifier &Instance()
	{
		static ReaderIdentifier sReaderIdentifier;
		return sReaderIdentifier;
	}

	/**
	 * @brief Initializes the reader identifier.
	 *
	 * @param identifier 32 byte reader identifier (reader_group_identifier | reader_group_sub_identifier).
	 *
	 * @return ALIRO_ERROR_NONE if the identifier was initialized successfully, otherwise an error code.
	 */
	AliroError Init(const GroupIdentifier &groupIdentifier, const SubIdentifier &subIdentifier);

	/**
	 * @brief Sets the reader group identifier.
	 *
	 * @param groupIdentifier 16 byte reader group identifier.
	 *
	 * @return ALIRO_ERROR_NONE if the group identifier was set successfully, otherwise an error code.
	 */
	AliroError SetGroupIdentifier(const GroupIdentifier &groupIdentifier);

	/**
	 * @brief Sets the reader group sub identifier.
	 *
	 * @param subIdentifier 16 byte reader group sub identifier.
	 *
	 * @return ALIRO_ERROR_NONE if the sub identifier was set successfully, otherwise an error code.
	 */
	AliroError SetSubIdentifier(const SubIdentifier &subIdentifier);

	/**
	 * @brief Returns 32 byte reader identifier (reader_group_identifier | reader_group_sub_identifier).
	 */
	const Identifier &Get() const;

private:
	/**
	 * @brief Default constructor for ReaderIdentifier.
	 */
	ReaderIdentifier() = default;

	/**
	 * @brief The ReaderIdentifier should not be cloneable.
	 */
	ReaderIdentifier(ReaderIdentifier &other) = delete;

	/**
	 * @brief The ReaderIdentifier should not be assignable.
	 */
	void operator=(const ReaderIdentifier &) = delete;

	/**
	 * @brief The ReaderIdentifier should not be movable.
	 */
	ReaderIdentifier(ReaderIdentifier &&) = delete;
	ReaderIdentifier &operator=(ReaderIdentifier &&) = delete;

	Identifier mIdentifier{ std::array<Byte, kReaderIdentifierLength>{
		0x37, 0x65, 0x20, 0x39, 0x31, 0x20, 0x61, 0x65, 0x20, 0x31, 0x64, 0x20, 0x33, 0x64, 0x20, 0x65,
		0x63, 0x20, 0x38, 0x36, 0x20, 0x31, 0x62, 0x20, 0x33, 0x39, 0x20, 0x31, 0x66, 0x20, 0x33, 0x34 } };
};

} // namespace Aliro
