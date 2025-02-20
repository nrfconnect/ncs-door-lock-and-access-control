/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include <cstddef>
#include <cstdint>
#include <initializer_list>

/**
 * @brief A simple dictionary for storing key-value pairs that can be brace-enclosed initialized.
 *
 * The user is accountable for using sane T1 and T2.
 * Make sure that these types implements operators and constructors required by FiniteMap.
 */
template <typename T1, typename T2, size_t N> struct FiniteMap {
private:
	using ElementCounterType = uint16_t;
	struct Item {
		T1 key{};
		T2 value{};
	};

public:
	constexpr FiniteMap() = default;
	constexpr FiniteMap(const std::initializer_list<Item> &list)
	{
		ElementCounterType idx{ 0 };
		for (auto &element : list) {
			mMap[idx++] = element;
		}
	}

	/* The user is responsible for making sure this operation makes sense under given key.
       Contains() can be used as a sanity checker. */
	const T2 &operator[](T1 key) const
	{
		static T2 dummyObject{};
		for (auto &it : mMap) {
			if (key == it.key)
				return it.value;
		}
		return dummyObject;
	}

	constexpr bool Contains(T1 key) const
	{
		for (const auto &it : mMap) {
			if (key == it.key)
				return true;
		}
		return false;
	}

private:
	Item mMap[N];
	ElementCounterType mElementsCount{ 0 };
};
