/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include <memory>
#include <new>
#include <utility>

namespace DoorLock::Utils {

/**
 * @brief Allocate and construct a single object without throwing on failure.
 *
 * @tparam T Type of the object to allocate.
 * @tparam Args Constructor argument types.
 * @param args Constructor arguments forwarded to @p T.
 *
 * @return Pointer to the allocated object, or `nullptr` on allocation failure.
 */
template <typename T, typename... Args> T *new_nothrow(Args &&...args) noexcept
{
	return new (std::nothrow) T(std::forward<Args>(args)...);
}

/**
 * @brief Allocate an array without throwing on failure.
 *
 * @tparam T Element type of the array.
 * @param n Number of elements to allocate.
 *
 * @return Pointer to the allocated array, or `nullptr` on allocation failure.
 */
template <typename T> T *new_array_nothrow(size_t n) noexcept
{
	return new (std::nothrow) T[n];
}

/**
 * @brief Create a `std::unique_ptr` to a single object using nothrow allocation.
 *
 * @tparam T Type of the object to allocate.
 * @tparam Args Constructor argument types.
 * @param args Constructor arguments forwarded to @p T.
 *
 * @return Owning smart pointer to the allocated object, or an empty pointer
 *         when allocation fails.
 */
template <typename T, typename... Args> std::unique_ptr<T> make_unique_nothrow(Args &&...args) noexcept
{
	return std::unique_ptr<T>(new_nothrow<T>(std::forward<Args>(args)...));
}

/**
 * @brief Create a `std::unique_ptr` to an array using nothrow allocation.
 *
 * @tparam T Element type of the array.
 * @param n Number of elements to allocate.
 *
 * @return Owning smart pointer to the allocated array, or an empty pointer
 *         when allocation fails.
 */
template <typename T> std::unique_ptr<T[]> make_unique_array_nothrow(size_t n) noexcept
{
	return std::unique_ptr<T[]>(new_array_nothrow<T>(n));
}

} // namespace DoorLock::Utils
