/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include <zephyr/settings/settings.h>

#include <cerrno>
#include <type_traits>

namespace DoorLock::SettingsUtils {

/**
 * @brief Save raw bytes under a settings key.
 *
 * @param keyName Fully-qualified settings key name.
 * @param data Pointer to bytes to store.
 * @param size Number of bytes to store.
 *
 * @retval 0 Success.
 * @retval <0 Error propagated from Zephyr settings backend.
 */
int Save(const char *keyName, const void *data, size_t size);

/**
 * @brief Load raw bytes stored under a settings key.
 *
 * @param keyName Fully-qualified settings key name.
 * @param data Destination buffer.
 * @param size [in] Destination buffer size, [out] loaded size or required size.
 *
 * @retval 0 Success.
 * @retval -ENOENT Key not found (or key exists but value is deleted).
 * @retval -ENOMEM Buffer is too small; @p size is updated to required size.
 * @retval <0 Other error propagated from Zephyr settings backend or read path.
 */
int Load(const char *keyName, void *data, size_t &size);

/**
 * @brief Load raw bytes, require an exact data size.
 *
 * @param keyName Fully-qualified settings key name.
 * @param data Destination buffer.
 * @param size Expected data size.
 *
 * @retval 0 Success.
 * @retval -EINVAL Stored value size differs from @p size.
 * @retval <0 Error from Load().
 */
int LoadExact(const char *keyName, void *data, size_t size);

/**
 * @brief Delete a settings key.
 *
 * @param keyName Fully-qualified settings key name.
 *
 * @retval 0 Success.
 * @retval <0 Error propagated from Zephyr settings backend.
 */
int Delete(const char *keyName);

/**
 * @brief Save a value of type @p T under a settings key.
 *
 * Stores the in-memory representation of @p value as-is.
 *
 * @tparam T Value type.
 * @param keyName Fully-qualified settings key name.
 * @param value Value to store.
 *
 * @retval 0 Success.
 * @retval <0 Error from raw Save().
 */
template <typename T> int Save(const char *keyName, const T &value)
{
	static_assert(std::is_trivially_copyable_v<T>, "T must be trivially copyable");
	return Save(keyName, &value, sizeof(T));
}

/**
 * @brief Load a value of type @p T from a settings key.
 *
 * This is equivalent to LoadExact(keyName, &value, sizeof(T)).
 *
 * @tparam T Value type.
 * @param keyName Fully-qualified settings key name.
 * @param value Destination value.
 *
 * @retval 0 Success.
 * @retval -EINVAL Stored value size does not match sizeof(T).
 * @retval <0 Error from LoadExact().
 */
template <typename T> int Load(const char *keyName, T &value)
{
	static_assert(std::is_trivially_copyable_v<T>, "T must be trivially copyable");
	return LoadExact(keyName, &value, sizeof(T));
}

} // namespace DoorLock::SettingsUtils
