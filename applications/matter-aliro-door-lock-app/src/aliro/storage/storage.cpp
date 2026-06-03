/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "storage.h"

#include <aliro/utils.h>
#include <settings_utils/settings_utils.h>

#include <zephyr/init.h>
#include <zephyr/logging/log.h>
#include <zephyr/settings/settings.h>

LOG_MODULE_REGISTER(storage, CONFIG_DOOR_LOCK_APP_LOG_LEVEL);

namespace {

int InitStorage(void)
{
	int status = settings_subsys_init();
	VerifyOrReturnStatus(status == 0, status, LOG_ERR("Initializing settings subsystem failed: %d", status));

	return 0;
}

} // namespace

int KeyValueStorage::Save(const char *keyName, const uint8_t *value, size_t valueLen)
{
	Aliro::StorageKeys::KeyNameBuffer key;
	snprintf(key.data(), key.size(), "%s/%s", Aliro::StorageKeys::kDoorLockBaseKey, keyName);

	return DoorLock::SettingsUtils::Save(key.data(), value, valueLen);
}

int KeyValueStorage::Clear(const char *keyName)
{
	Aliro::StorageKeys::KeyNameBuffer key;
	snprintf(key.data(), key.size(), "%s/%s", Aliro::StorageKeys::kDoorLockBaseKey, keyName);

	return DoorLock::SettingsUtils::Delete(key.data());
}

int KeyValueStorage::Get(const char *keyName, uint8_t *buf, size_t bufLength)
{
	Aliro::StorageKeys::KeyNameBuffer key;
	snprintf(key.data(), key.size(), "%s/%s", Aliro::StorageKeys::kDoorLockBaseKey, keyName);

	return DoorLock::SettingsUtils::LoadExact(key.data(), buf, bufLength);
}

Aliro::StorageKeys::KeyNameBuffer KeyValueStorage::GetStorageKeyName(KeyIdString keyIdString, size_t keyId)
{
	Aliro::StorageKeys::KeyNameBuffer keyName;
	snprintf(keyName.data(), keyName.size(), "%s/%u", keyIdString, keyId);
	return keyName;
}

SYS_INIT(InitStorage, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);
