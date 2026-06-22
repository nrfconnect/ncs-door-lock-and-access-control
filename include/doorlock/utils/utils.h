/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include <zephyr/sys/__assert.h>

/**
 *  @def VerifyOrReturnValue(expr, value, ...)
 *
 *  @brief
 *    Returns a specified status code if expression evaluates to false.
 *
 *  @param[in]  expr        A Boolean expression to be evaluated.
 *  @param[in]  value       A value to return if @a expr is false.
 *  @param[in]  ...         Statements to execute before returning. Optional. Good for logging.
 *
 */
#define VerifyOrReturnValue(expr, value, ...)                                                                          \
	do {                                                                                                           \
		if (!(expr)) {                                                                                         \
			__VA_ARGS__;                                                                                   \
			return (value);                                                                                \
		}                                                                                                      \
	} while (false)

/**
 *  @brief Aliases for VerifyOrReturnValue().
 */
#define VerifyOrReturnFalse(expr, ...) VerifyOrReturnValue(expr, false, ##__VA_ARGS__)
#define VerifyOrReturnTrue(expr, ...) VerifyOrReturnValue(expr, true, ##__VA_ARGS__)

/**
 *  @def VerifyOrReturn(expr, ...)
 *
 *  @brief
 *    Returns from the function if expression evaluates to false.
 *
 *  @param[in]  expr        A Boolean expression to be evaluated.
 *  @param[in]  ...         Statements to execute before returning. Optional. Good for logging.
 */
#define VerifyOrReturn(expr, ...)                                                                                      \
	do {                                                                                                           \
		if (!(expr)) {                                                                                         \
			__VA_ARGS__;                                                                                   \
			return;                                                                                        \
		}                                                                                                      \
	} while (false)

/**
 *  @def VerifyOrExit(expr, ...)
 *
 *  @brief
 *    Goes to exit if expression evaluates to false.
 *
 *  @param[in]  expr        A Boolean expression to be evaluated.
 *  @param[in]  ...         Statements to execute before exiting. Optional. Good for logging.
 */
#define VerifyOrExit(expr, ...)                                                                                        \
	do {                                                                                                           \
		if (!(expr)) {                                                                                         \
			__VA_ARGS__;                                                                                   \
			goto exit;                                                                                     \
		}                                                                                                      \
	} while (false)

/**
 *  @def VerifyOrDie(expr, msg)
 *
 *  @brief
 *    Asserts if expression evaluates to false.
 *
 *  @param[in]  expr        A Boolean expression to be evaluated.
 *  @param[in]  msg         A message to be passed to __ASSERT.
 */
#define VerifyOrDie(expr, msg)                                                                                         \
	do {                                                                                                           \
		if (!(expr)) {                                                                                         \
			__ASSERT(false, msg);                                                                          \
		}                                                                                                      \
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
#define VerifyAndCall(clb, ...)                                                                                        \
	do {                                                                                                           \
		if (clb) {                                                                                             \
			clb(__VA_ARGS__);                                                                              \
		}                                                                                                      \
	} while (false)

#ifdef __cplusplus

#include <type_traits>

namespace DoorLock {
namespace Utils {

/**
 * @brief Equivalent of C++23 std::to_underlying.
 */
template <class T> constexpr std::underlying_type_t<T> ToUnderlying(T e)
{
	static_assert(std::is_enum<T>::value, "ToUnderlying called on non-enum value.");
	return static_cast<std::underlying_type_t<T>>(e);
}

} // namespace Utils
} // namespace DoorLock

#endif
