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

	static ReaderIdentifier &Instance()
	{
		static ReaderIdentifier sReaderIdentifier;
		return sReaderIdentifier;
	}

	/**
	 * @brief Returns 32 byte reader identifier (reader_group_identifier | reader_group_sub_identifier).
	 */
	const Identifier &Get();

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
};

} // namespace Aliro
