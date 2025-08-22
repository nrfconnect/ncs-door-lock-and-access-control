/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include <cerrno>
#include <cstddef>
#include <cstdint>

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
	int Save(const char *keyName, const uint8_t *value, size_t valueLen);

	/**
	 * @brief Clears a single key by name from persisent storage.
	 *
	 * @param keyName input name of a key to clear.
	 *
	 * @return 0 when success, an error code otherwise.
	 */
	int Clear(const char *keyName);

	/**
	 * @brief Gets a single key by name from persisent storage.
	 *
	 * @param keyName input name of key to get.
	 * @param buf input buffer with value.
	 * @param bufLength input length of the buffer.
	 *
	 * @return 0 when key has read, the -ENODATA when key does not exist or error code otherwise.
	 */
	int Get(const char *keyName, uint8_t *buf, size_t bufLength);

private:
	KeyValueStorage() = default;
	KeyValueStorage(KeyValueStorage &other) = delete;
	void operator=(const KeyValueStorage &) = delete;
	KeyValueStorage(KeyValueStorage &&) = delete;
	KeyValueStorage &operator=(KeyValueStorage &&) = delete;
};
