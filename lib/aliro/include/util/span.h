/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include "errors.h"
#include "static_byte_span.h"
#include <cstring>

using Byte = uint8_t;
using Ptr = Byte *;

/**
 * @brief A class that represents the span of bytes allocated inside of the object of this class.
 */
class SharedByteSpan {
public:
	friend class ByteSpan;
	constexpr SharedByteSpan() = default;

	SharedByteSpan(Ptr data, size_t length);

	// copy semantics
	SharedByteSpan(const SharedByteSpan &other);
	SharedByteSpan &operator=(const SharedByteSpan &other);
	template <Capacity C> SharedByteSpan &operator=(const StaticByteSpan<C> &other)
	{
		Assign(other);
		return *this;
	}

	// move semantics
	SharedByteSpan(SharedByteSpan &&other);
	SharedByteSpan &operator=(SharedByteSpan &&other);

	SharedByteSpan operator+(const SharedByteSpan &other);
	SharedByteSpan operator+=(const SharedByteSpan &other);

	template <Capacity C> SharedByteSpan operator+(const StaticByteSpan<C> &other)
	{
		return Append(other, other.Size());
	}

	template <Capacity C> SharedByteSpan operator+=(const StaticByteSpan<C> &other)
	{
		return Append(other, other.Size());
	}

	operator bool() const { return (mBuffer && mCapacity); }

	~SharedByteSpan() { Free(); }

	/* Accessors. */
	constexpr Ptr Data() const { return mBuffer; }
	constexpr Ptr Begin() const { return Data(); }
	constexpr Ptr End() const { return Data() + Size(); }
	constexpr size_t Size() const { return mLength; }
	constexpr bool Empty() const { return (!mBuffer || Size() == 0); }

	AliroError Assign(const Byte *data, size_t length);
	AliroError Assign(const SharedByteSpan &span) { return Assign(span.Data(), span.Size()); }
	template <Capacity C> AliroError Assign(const StaticByteSpan<C> &staticByteSpan)
	{
		return Assign(staticByteSpan.Data(), staticByteSpan.Size());
	}

	AliroError Resize(size_t newCapacity, bool doNotRelease = false);

	template <typename T> bool Equal(const T *otherData, size_t otherSize)
	{
		return (Size() == otherSize) && (Empty() || (memcmp(Data(), otherData, Size() * sizeof(T)) == 0));
	}

	bool Equal(const SharedByteSpan &span) { return Equal(span.Data(), span.Size()); }

	/**
	 * @brief Explicitly release memory.
	 */
	void Release();

private:
	template <typename T> SharedByteSpan Append(T &other, size_t dataLength)
	{
		size_t newCapacity = this->mCapacity + dataLength;
		Ptr newDataBuffer = new Byte[newCapacity];
		VerifyOrDie(newDataBuffer, "No memory");
		memcpy(newDataBuffer, this->Begin(), this->mCapacity);
		memcpy(newDataBuffer + this->mCapacity, other.Begin(), dataLength);
		mLength = this->Size() + dataLength;
		Free();
		mBuffer = newDataBuffer;
		mCapacity = newCapacity;

		return *this;
	}

	void Free();

	Ptr mBuffer{ nullptr };
	size_t mCapacity{ 0 };
	size_t mLength{ 0 };
};

/**
 * @brief A class that represents a shallow copy of a span of bytes allocated in SharedByteSpan.
 */
class ByteSpan {
public:
	ByteSpan() : mBuffer(nullptr), mLength(0) {}
	ByteSpan(Ptr data, uint8_t length) : mBuffer(data), mLength(length) {}
	ByteSpan(const ByteSpan &other);

	const ByteSpan &operator=(const ByteSpan &other);
	/* Enables to carry the reference to data stored in SharedByteSpan */
	const ByteSpan &operator=(const SharedByteSpan &other);

	Byte operator[](size_t index) { return *(mBuffer + index); }

	operator bool() const { return (mBuffer && mLength); }

	/* For now, no move. */
	ByteSpan(ByteSpan &&other) = delete;
	const ByteSpan &operator=(ByteSpan &&other) = delete;

	~ByteSpan();

	/* Accessors. */
	constexpr Ptr Data() const { return mBuffer; }
	constexpr Ptr Begin() const { return Data(); }
	constexpr Ptr End() const { return Data() + Size(); }
	constexpr size_t Size() const { return mLength; }
	constexpr bool Empty() const { return (Size() == 0); }

private:
	Ptr mBuffer;
	size_t mLength;
};
