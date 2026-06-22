/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

#pragma once

#include <zephyr/sys/__assert.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <type_traits>

// clang-format off
/**
 *  @def VerifyOrReturnStatus(expr, value, ...)
 *
 *  @brief
 *    Returns a specified status code if expression evaluates to false.
 *
 *  @param[in]  expr        A Boolean expression to be evaluated.
 *  @param[in]  value       A value to return if @a expr is false.
 *  @param[in]  ...         Statements to execute before returning. Optional. Good for logging.
 *
 */
#define VerifyOrReturnStatus(expr, value, ...)  \
	do {                                        \
		if (!(expr)) {                          \
			__VA_ARGS__;                        \
			return (value);                     \
		}                                       \
	} while (false)

/**
 *  @brief Aliases for VerifyOrReturnStatus().
 */
#define VerifyOrReturnValue(expr, value, ...)  VerifyOrReturnStatus(expr, value, ##__VA_ARGS__)
#define VerifyOrReturnFalse(expr, ...)  VerifyOrReturnStatus(expr, false, ##__VA_ARGS__)
#define VerifyOrReturnTrue(expr, ...)  VerifyOrReturnStatus(expr, true, ##__VA_ARGS__)

/**
 *  @def VerifyOrReturn(expr, ...)
 *
 *  @brief
 *    Returns from the function if expression evaluates to false.
 *
 *  @param[in]  expr        A Boolean expression to be evaluated.
 *  @param[in]  ...         Statements to execute before returning. Optional. Good for logging.
 */
// clang-format off
#define VerifyOrReturn(expr, ...)               \
	do {                                        \
		if (!(expr)) {                          \
			__VA_ARGS__;                        \
			return;                             \
		}                                       \
	} while (false)

/**
 *  @def VerifyOrExit(expr, value, ...)
 *
 *  @brief
 *    Goes to exit if expression evaluates to false.
 *
 *  @param[in]  expr        A Boolean expression to be evaluated.
 *  @param[in]  ...         Statements to execute before exiting. Optional. Good for logging.
 */
#define VerifyOrExit(expr, ...)  \
	do {                         \
		if (!(expr)) {           \
			__VA_ARGS__;         \
			goto exit;           \
		}                        \
	} while (false)

/**
 *  @def VerifyAndExit(expr, value, ...)
 *
 *  @brief
 *    Goes to exit if expression evaluates to true.
 *
 *  @param[in]  expr        A Boolean expression to be evaluated.
 *  @param[in]  ...         Statements to execute before exiting. Optional. Good for logging.
 */
#define VerifyAndExit(expr, ...)  \
	do {                         \
		if (expr) {              \
			__VA_ARGS__;         \
			goto exit;           \
		}                        \
	} while (false)

/**
 *  @def ReturnErrorOnFailure(expr)
 *
 *  @brief
 *    Returns the error code if the expression returns an error, this means any value other
 *    than ALIRO_NO_ERROR.
 *
 *  Example usage:
 *
 *  @param[in]  __expr        An expression to be tested.
 */
#define ReturnErrorOnFailure(__expr)       \
	do {                                   \
		auto errorCode = (__expr);         \
		if (errorCode != ALIRO_NO_ERROR) { \
			return errorCode;              \
		}                                  \
	} while (false)

/**
 *  @def VerifyOrDie(expr, value, ...)
 *
 *  @brief
 *    Goes to exit if expression evaluates to false.
 *
 *  @param[in]  expr        A Boolean expression to be evaluated.
 */
#define VerifyOrDie(expr, msg)                 \
	do {                                       \
		if (!(expr)) {                         \
			__ASSERT(false, msg);              \
		}                                      \
	} while (false)

/**
 *  @def VerifyAndCall(clb, ...)
 *
 *  @brief
 *    Calls a specific callback if it is valid.
 *
 *  @param[in]  clb        A callback to be evaluated and called.
 *  @param[in]  ...        Arguments to be passed to the callback.
 */
#define VerifyAndCall(clb, ...) \
	do {                        \
		if (clb) {              \
			clb(__VA_ARGS__);   \
		}                       \
	} while (false)

// clang-format on

/**
 * @brief Equivalent of C++23 std::to_underlying.
 */
template <class T> constexpr std::underlying_type_t<T> ToUnderlying(T e)
{
	static_assert(std::is_enum<T>::value, "ToUnderlying called on non-enum value.");
	return static_cast<std::underlying_type_t<T>>(e);
}
