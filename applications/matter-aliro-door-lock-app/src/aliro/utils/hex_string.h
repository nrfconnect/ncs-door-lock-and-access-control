/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <tuple>
#include <type_traits>
#include <zephyr/sys/util.h>

namespace DoorLock::Utils {

namespace Detail {

template <typename T> using RemoveCvRefType = std::remove_cv_t<std::remove_reference_t<T>>;

template <typename T> struct IsByteArray : std::false_type {};

template <size_t N> struct IsByteArray<std::array<uint8_t, N>> : std::true_type {};

template <typename T> inline constexpr bool kIsByteArray = IsByteArray<RemoveCvRefType<T>>::value;

template <typename TObject> constexpr size_t GetArraySize()
{
	using ObjectType = RemoveCvRefType<TObject>;
	static_assert(kIsByteArray<ObjectType>, "Object type must be std::array<uint8_t, N>");
	return std::tuple_size<ObjectType>::value;
}

template <typename TObject> struct HexStringBufferType;

template <size_t N> struct HexStringBufferType<std::array<uint8_t, N>> {
	using Type = std::array<char, N * 2 + 1>;
};

} // namespace Detail

/**
 * @brief Hex string buffer type for a std::array object type.
 *
 * Produces a null-terminated character buffer large enough to hold the
 * hexadecimal representation of @p TObject:
 * - 2 characters per byte
 * - plus 1 character for the null terminator
 *
 * @tparam TObject `std::array<uint8_t, N>` object type to be formatted.
 */
template <typename TObject>
using HexStringBuffer = typename Detail::HexStringBufferType<Detail::RemoveCvRefType<TObject>>::Type;

/**
 * @brief Convert a std::array object to a hexadecimal C-string.
 *
 * The function formats up to @p length bytes from @p object into @p buffer
 * using `bin2hex`. If @p length is omitted, it defaults to the number of
 * elements in the array (`N` for `std::array<uint8_t, N>`).
 *
 * Compile-time checks enforce:
 * - @p TObject is `std::array<uint8_t, N>`
 * - @p TBuffer matches `HexStringBuffer<TObject>`
 *
 * @tparam TBuffer Character buffer type, expected to be `HexStringBuffer<TObject>`.
 * @tparam TObject Input object type, expected to be `std::array<uint8_t, N>`.
 * @param buffer Output hex string buffer.
 * @param object Input data object to format.
 * @param length Number of bytes to print. Defaults to `N`.
 *
 * @retval true Conversion completed and produced exactly `length * 2` characters.
 * @retval false `length` exceeds object byte size or conversion result is invalid.
 */
template <typename TBuffer, typename TObject>
inline bool ArrayToHexString(TBuffer &buffer, const TObject &object, size_t length = Detail::GetArraySize<TObject>())
{
	using ObjectType = Detail::RemoveCvRefType<TObject>;
	using BufferType = Detail::RemoveCvRefType<TBuffer>;

	static_assert(Detail::kIsByteArray<ObjectType>, "Object type must be std::array<uint8_t, N>");
	static_assert(std::is_same_v<BufferType, HexStringBuffer<ObjectType>>,
		      "Buffer type must match the object hex buffer type");

	if (length > Detail::GetArraySize<ObjectType>()) {
		return false;
	}

	const size_t convertedLength = bin2hex(object.data(), length, buffer.data(), buffer.size());
	return convertedLength == length * 2;
}

} // namespace DoorLock::Utils
