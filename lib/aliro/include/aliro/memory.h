/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

#pragma once

#include <memory>
#include <new>

namespace Aliro {

template <typename T, typename... Args> T *new_nothrow(Args &&...args) noexcept
{
	return new (std::nothrow) T(std::forward<Args>(args)...);
}

template <typename T> T *new_array_nothrow(std::size_t n) noexcept
{
	return new (std::nothrow) T[n];
}

template <typename T, typename... Args> std::unique_ptr<T> make_unique_nothrow(Args &&...args) noexcept
{
	return std::unique_ptr<T>(new_nothrow<T>(std::forward<Args>(args)...));
}

} // namespace Aliro
