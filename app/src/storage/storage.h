/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include <storage_keys.h>

#include <util/static_byte_span.h>

#include <string>

/**
 * @class KeyValueStorage
 * @brief A class for simple key-value persistent storage representing.
 */
class KeyValueStorage {
public:
	static KeyValueStorage &Instance()
	{
		static KeyValueStorage sKeyValueStorage;
		return sKeyValueStorage;
	}

	/**
	 * @brief Writes a single key and value to persistend storage.
	 * When the key exists the value is saved if it has changed.
	 *
	 * @param keyName input name of key.
	 * @param value input buffer with value.
	 * @param valueLen input length of the buffer.
	 *
	 * @return 0 when success, a error code otherwise.
	 */
	int Save(const std::string &keyName, const uint8_t *value, size_t valueLen);

	/**
	 * @brief Gets a single key by name from persisent storage.
	 *
	 * @param keyName input name of key to get.
	 * @param buf input buffer with value.
	 * @param bufLength input length of the buffer.
	 *
	 * @return 0 when key has read, the -ENODATA when key does not exist or error code otherwise.
	 */
	int Get(const std::string &keyName, uint8_t *buf, size_t bufLength);

	/**
	 * @brief Gets a single key by name from persisent storage.
	 *
	 * @param keyName input name of key to get.
	 * @param value output buffer with value.
	 *
	 * @return 0 when key has read, the -ENODATA when key does not exist or error code otherwise.
	 */
	template <size_t N> int Get(const std::string &keyName, StaticByteSpan<N> &value)
	{
		std::array<Byte, N> valueBuffer{};
		int erc = Get(keyName, valueBuffer.data(), valueBuffer.size());
		VerifyOrReturnValue(erc == 0, erc);

		VerifyOrReturnValue(value.Set(valueBuffer.data(), valueBuffer.size()) == ALIRO_NO_ERROR, -EIO);
		return 0;
	}

private:
	KeyValueStorage() = default;
	KeyValueStorage(KeyValueStorage &other) = delete;
	void operator=(const KeyValueStorage &) = delete;
	KeyValueStorage(KeyValueStorage &&) = delete;
	KeyValueStorage &operator=(KeyValueStorage &&) = delete;
};
