/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include <cstdint>

namespace Aliro::Uwb::Utils {

/**
 * @brief Parts of a milli value.
 *
 * @param sign The sign of the milli value.
 * @param integer The integer part of the milli value.
 * @param fraction The fraction part of the milli value.
 */
struct MilliParts {
	const char *mSign;
	uint32_t mInteger;
	uint32_t mFraction;
};

/**
 * @brief Splits a milli value into parts.
 *
 * @param milli The milli value to split.
 *
 * @return The parts of the milli value.
 */
constexpr inline MilliParts SplitMilli(int32_t milli) noexcept
{
	if (milli < 0) {
		const uint32_t pos = static_cast<uint32_t>(-static_cast<int64_t>(milli));
		return { "-", pos / 1000U, pos % 1000U };
	}

	const uint32_t pos = static_cast<uint32_t>(milli);
	return { "", pos / 1000U, pos % 1000U };
}

} // namespace Aliro::Uwb::Utils
