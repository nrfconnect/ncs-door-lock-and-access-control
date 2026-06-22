/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <settings_utils/settings_utils.h>

#include <zephyr/logging/log.h>
#include <zephyr/settings/settings.h>

LOG_MODULE_REGISTER(settings_utils, CONFIG_DOOR_LOCK_SETTINGS_UTILS_LOG_LEVEL);

namespace DoorLock::SettingsUtils {

int Save(const char *keyName, const void *data, size_t size)
{
	int err = settings_save_one(keyName, data, size);
	if (err != 0) {
		LOG_ERR("Failed to save key: %s, error: %d", keyName, err);
		return err;
	}

	return 0;
}

int Load(const char *keyName, void *data, size_t &size)
{
	const ssize_t result = settings_load_one(keyName, data, size);
	if (result < 0) {
		const int err = static_cast<int>(result);
		LOG_ERR("Failed to load key: %s, error: %d", keyName, err);
		size = 0;
		return err;
	}

	const size_t readLength = static_cast<size_t>(result);

	if (readLength == 0) {
		LOG_DBG("Key not found: %s", keyName);
		size = 0;
		return -ENOENT;
	}

	if (readLength > size) {
		LOG_ERR("Load failed: buffer too small: %zu, actual size: %zu", size, readLength);
		size = readLength;
		return -ENOMEM;
	}

	LOG_DBG("Loaded key: %s, buffer size: %zu, read length: %zu", keyName, size, readLength);
	size = readLength;
	return 0;
}

int LoadExact(const char *keyName, void *data, size_t size)
{
	size_t actualSize{ size };
	const int rc = Load(keyName, data, actualSize);
	if (rc != 0) {
		return rc;
	}

	if (actualSize != size) {
		LOG_ERR("LoadExact failed: expected %zu, got %zu", size, actualSize);
		return -EINVAL;
	}

	return 0;
}

int Delete(const char *keyName)
{
	int err = settings_delete(keyName);
	if (err != 0) {
		LOG_ERR("Failed to delete key: %s, error: %d", keyName, err);
		return err;
	}

	return 0;
}

} // namespace DoorLock::SettingsUtils
