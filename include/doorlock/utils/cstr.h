/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include <cstddef>

namespace DoorLock::Utils {

/**
 * @brief Returns the compile-time size of a C-string array.
 *
 * This helper accepts only `const char[N]` arrays and returns `N`, which
 * includes the trailing null terminator.
 *
 * @tparam N Size of the character array.
 * @param[in] str Character array (typically a string literal).
 *
 * @return The array size, including the null terminator.
 */
template <size_t N> constexpr size_t CStrSize(const char (&str)[N])
{
	(void)str;
	return N;
}

/**
 * @brief Returns the compile-time length of a C-string array.
 *
 * This helper accepts only `const char[N]` arrays and returns `N - 1`, which
 * excludes the trailing null terminator.
 *
 * @tparam N Size of the character array.
 * @param[in] str Character array (typically a string literal).
 *
 * @return The string length, excluding the null terminator.
 */
template <size_t N> constexpr size_t CStrLen(const char (&str)[N])
{
	(void)str;
	return N - 1;
}

} // namespace DoorLock::Utils
