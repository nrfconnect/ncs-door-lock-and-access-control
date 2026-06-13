/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "counter.h"

#include <cstdio>
#include <tuple>

#include <psa/crypto.h>
#include <psa/protected_storage.h>
#include <zephyr/logging/log.h>
#include <zephyr/settings/settings.h>

LOG_MODULE_DECLARE(external_nvs, CONFIG_DOOR_LOCK_EXTERNAL_NVS_LOG_LEVEL);

namespace DoorLock::ExternalNvs::Counter {

namespace {

static_assert((kUniqueIdSize % 4) == 0, "Unique ID size must be a multiple of 4");

/** PSA Protected Storage UID for the device Unique ID (internal flash). */
constexpr psa_storage_uid_t kPsUniqueIdUid{ CONFIG_DOOR_LOCK_EXTERNAL_NVS_ROLLBACK_PROTECTION_UNIQUE_ID_UID };

constexpr auto kSettingsPrefix{ CONFIG_DOOR_LOCK_EXTERNAL_NVS_ROLLBACK_PROTECTION_SETTINGS_PREFIX };

using KeyBuffer = std::array<char, SETTINGS_MAX_NAME_LEN + 1>;

struct DeleteSubtreeEntry {
	const char *prefix;
	int result;
};

UniqueId sUniqueId;
bool sInitialized{ false };

int GenerateUniqueId(UniqueId &uniqueId)
{
	auto status = psa_crypto_init();
	if (status != PSA_SUCCESS) {
		LOG_ERR("Failed to initialize PSA Crypto: %d", status);
		return -ENODEV;
	}

	status = psa_generate_random(uniqueId.data(), uniqueId.size());
	if (status != PSA_SUCCESS) {
		LOG_ERR("Failed to generate random unique ID: %d", status);
		return -EIO;
	}

	return 0;
}

int PrepareCounterKey(KeyBuffer &keyBuffer, Id id)
{
	auto err = snprintf(keyBuffer.data(), keyBuffer.size(), "%s/%u", kSettingsPrefix, id);
	if (err < 0 || static_cast<size_t>(err) >= keyBuffer.size()) {
		LOG_ERR("Failed to format counter key");
		return -ENOMEM;
	}

	return 0;
}

int DeleteSubtreeCallback(const char *name, size_t entrySize, settings_read_cb readCb, void *cbArg, void *param)
{
	ARG_UNUSED(entrySize);
	ARG_UNUSED(readCb);
	ARG_UNUSED(cbArg);

	DeleteSubtreeEntry &entry = *static_cast<DeleteSubtreeEntry *>(param);
	KeyBuffer keyBuffer;

	// name comes from Zephyr settings subsystem so it is guaranteed to fit in the buffer.
	std::ignore = snprintf(keyBuffer.data(), keyBuffer.size(), "%s/%s", entry.prefix, name);
	const int result = settings_delete(keyBuffer.data());

	// Return the first error, but continue removing remaining keys anyway.
	if (entry.result == 0) {
		entry.result = result;
	}

	return 0;
}

int PsaStatusToErrno(psa_status_t status)
{
	switch (status) {
	case PSA_SUCCESS:
		return 0;
	case PSA_ERROR_DOES_NOT_EXIST:
		return -ENOENT;
	case PSA_ERROR_INVALID_ARGUMENT:
		return -EINVAL;
	case PSA_ERROR_INSUFFICIENT_STORAGE:
		return -ENOSPC;
	default:
		return -EIO;
	}
}

} // namespace

int Init()
{
	auto err = settings_subsys_init();
	if (err != 0) {
		LOG_ERR("Failed to initialize settings subsystem: %d", err);
		return err;
	}

	size_t readLength{ 0 };
	auto status = psa_ps_get(kPsUniqueIdUid, 0, sUniqueId.size(), sUniqueId.data(), &readLength);
	if (status == PSA_ERROR_DOES_NOT_EXIST) {
		LOG_INF("Unique ID not found in PSA Protected Storage, generating and storing");
		err = GenerateUniqueId(sUniqueId);
		if (err != 0) {
			return err;
		}
		status = psa_ps_set(kPsUniqueIdUid, sUniqueId.size(), sUniqueId.data(), PSA_STORAGE_FLAG_NONE);
		if (status != PSA_SUCCESS) {
			LOG_ERR("Failed to write Unique ID to PSA Protected Storage: %d", status);
			return PsaStatusToErrno(status);
		}
	} else if (status != PSA_SUCCESS) {
		LOG_ERR("Failed to read Unique ID from PSA Protected Storage: %d", status);
		return PsaStatusToErrno(status);
	} else if (readLength != sUniqueId.size()) {
		LOG_ERR("Unique ID length mismatch, expected %zu, got %zu", sUniqueId.size(), readLength);
		return -EIO;
	}

	sInitialized = true;

	return 0;
}

int Clear()
{
	sInitialized = false;

	auto status = psa_ps_remove(kPsUniqueIdUid);
	if (status != PSA_SUCCESS && status != PSA_ERROR_DOES_NOT_EXIST) {
		LOG_ERR("Failed to remove Unique ID from PSA Protected Storage: %d", status);
		return PsaStatusToErrno(status);
	}

	DeleteSubtreeEntry entry{ kSettingsPrefix, 0 };
	auto err = settings_load_subtree_direct(kSettingsPrefix, DeleteSubtreeCallback, &entry);
	if (err != 0) {
		LOG_ERR("Failed to delete subtree: %d", err);
		return err;
	}

	if (entry.result != 0) {
		LOG_ERR("Failed to delete subtree, result: %d", entry.result);
		return entry.result;
	}

	return 0;
}

int GetUniqueId(UniqueId &uniqueId)
{
	if (!sInitialized) {
		LOG_ERR("Counter is not initialized");
		return -ENODEV;
	}

	uniqueId = sUniqueId;

	return 0;
}

int Read(Id id, Counter &counter)
{
	KeyBuffer keyBuffer;

	auto err = PrepareCounterKey(keyBuffer, id);
	if (err != 0) {
		return err;
	}

	err = settings_load_one(keyBuffer.data(), &counter, sizeof(counter));
	if (err == -ENOENT || err == 0) {
		LOG_INF("Counter not found");
		counter = 0;
		return 0;
	} else if (err < 0) {
		LOG_ERR("Failed to load counter: %d", err);
		return err;
	} else if (err != sizeof(counter)) {
		LOG_ERR("Counter length mismatch, expected %zu, got %d", sizeof(counter), err);
		return -EIO;
	}

	return 0;
}

int Write(Id id, const Counter &counter)
{
	KeyBuffer keyBuffer;

	auto err = PrepareCounterKey(keyBuffer, id);
	if (err != 0) {
		return err;
	}

	err = settings_save_one(keyBuffer.data(), &counter, sizeof(counter));
	if (err != 0) {
		LOG_ERR("Failed to save counter: %d", err);
		return err;
	}

	return 0;
}

} // namespace DoorLock::ExternalNvs::Counter
