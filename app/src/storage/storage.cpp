/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "storage.h"
#include "aliro/utils.h"
#include "storage_keys.h"

#include <zephyr/init.h>
#include <zephyr/logging/log.h>
#include <zephyr/settings/settings.h>

LOG_MODULE_REGISTER(storage, CONFIG_NCS_DOOR_LOCK_APP_LOG_LEVEL);

struct LoadItemParam {
	uint8_t *buf;
	size_t bufLen;
	bool found;
};

static int LoadItemCallback(const char *, size_t len, settings_read_cb read_cb, void *cb_arg, void *param)
{
	LoadItemParam &itemParam = *static_cast<LoadItemParam *>(param);

	VerifyOrReturnStatus(len == itemParam.bufLen, -EINVAL, LOG_ERR("Invalid item length"));

	size_t lengthRead = read_cb(cb_arg, itemParam.buf, itemParam.bufLen);

	VerifyOrReturnStatus(lengthRead == len, -EIO, LOG_ERR("Item read failed"));

	itemParam.found = true;
	return 1;
}

int KeyValueStorage::Save(const char *keyName, const uint8_t *value, size_t valueLen)
{
	Aliro::StorageKeys::KeyNameBuffer key;
	snprintf(key.data(), key.size(), "%s/%s", Aliro::StorageKeys::kDoorLockBaseKey, keyName);

	return settings_save_one(key.data(), value, valueLen);
}

int KeyValueStorage::Clear(const char *keyName)
{
	Aliro::StorageKeys::KeyNameBuffer key;
	snprintf(key.data(), key.size(), "%s/%s", Aliro::StorageKeys::kDoorLockBaseKey, keyName);

	return settings_delete(key.data());
}

int KeyValueStorage::Get(const char *keyName, uint8_t *buf, size_t bufLength)
{
	Aliro::StorageKeys::KeyNameBuffer key;
	snprintf(key.data(), key.size(), "%s/%s", Aliro::StorageKeys::kDoorLockBaseKey, keyName);

	LoadItemParam param{ buf, bufLength, false };
	int status = settings_load_subtree_direct(key.data(), LoadItemCallback, &param);
	VerifyOrReturnStatus(status == 0, status, LOG_ERR("Loading subtree failed: %d", status));

	return (param.found ? 0 : -ENODATA);
}

static int InitStorage(void)
{
	int status = settings_subsys_init();
	VerifyOrReturnStatus(status == 0, status, LOG_ERR("Initializing settings subsystem failed: %d", status));

	return 0;
}

SYS_INIT(InitStorage, POST_KERNEL, CONFIG_APPLICATION_INIT_PRIORITY);
