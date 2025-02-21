/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include "errors.h"
#include "utils.h"

#include <array>

using Byte = uint8_t;
using Capacity = size_t;

template <Capacity C> class StaticByteSpan {
	using pointer = Byte *;
	using const_pointer = const Byte *;

public:
	StaticByteSpan() : mCurrentDataLength(0) {};
	StaticByteSpan(const std::array<Byte, C> &array) : mData(array), mCurrentDataLength(array.size()) {};

	StaticByteSpan(const_pointer data, size_t dataLength)
	{
		VerifyOrDie(Set(data, dataLength) == ALIRO_NO_ERROR, "Cannot set data");
	}

	StaticByteSpan(const StaticByteSpan &other) : mCurrentDataLength(other.mCurrentDataLength)
	{
		// Copy only necessary amount of data.
		memcpy(mData.data(), other.mData.data(), mCurrentDataLength);
	}

	~StaticByteSpan() { mCurrentDataLength = 0; }

	StaticByteSpan &operator=(const StaticByteSpan &other)
	{
		mCurrentDataLength = other.mCurrentDataLength;
		memcpy(mData.data(), other.mData.data(), mCurrentDataLength);

		return *this;
	}

	bool operator==(const StaticByteSpan &other) const
	{
		return (mCurrentDataLength == other.mCurrentDataLength) &&
		       memcmp(mData.data(), other.mData.data(), mCurrentDataLength) == 0;
	}

	// Returns the number of elements in container.
	size_t Size() const { return mCurrentDataLength; }

	// Returns the number of elements that can be held in current allocated storage.
	size_t Capacity() const { return mData.max_size(); }

	// Check whether the container is empty.
	bool Empty() const { return Size() == 0; }

	// Direct access to the data.
	const_pointer Data() const { return mData.data(); }

	// Returns a pointer to the beginning of data.
	const_pointer Begin() const { return mData.begin(); }
	pointer Begin() { return mData.begin(); }

	// Returns a pointer to the end of valid data (past-the-last element).
	const_pointer End() const { return Begin() + Size(); }

	// Returns an pointer to the end of container.
	const_pointer ContainerEnd() const { return mData.end(); }

	/**
	 * Obtains a const element of the container given an index.
	 * NOTE: When index is out of bounds the value is unknown - it is illegal to set index out of bounds.
	 * The Size() must be checked by User prior to indexing the container.
	 */
	Byte operator[](size_t index) const { return mData[index]; }

	// When it is possible, appends the copy of value to the end of current position in container.
	void Append(const Byte &value)
	{
		if ((Begin() + mCurrentDataLength + 1) > ContainerEnd()) {
			return;
		}
		mData[mCurrentDataLength++] = value;
	}

	/**
	 * Set elements from `data` at the beggining of the container.
	 * After successfuly set new elements the Size() returns total number of elements in container.
	 */
	AliroError Set(const_pointer data, size_t dataLength)
	{
		VerifyOrReturnStatus(dataLength > 0 && data != nullptr, ALIRO_INVALID_ARGUMENT);
		VerifyOrReturnStatus(dataLength <= Capacity(), ALIRO_NO_MEMORY);

		memcpy(mData.data(), data, dataLength);
		mCurrentDataLength = dataLength;

		return ALIRO_NO_ERROR;
	}

	/**
	 * Returns a copy of the data as a std::array.
	 *
	 * @return The copy of the data as the std::array.
	 */
	std::array<Byte, C> ToArray() const { return mData; }

private:
	std::array<Byte, C> mData{};
	size_t mCurrentDataLength{};
};
