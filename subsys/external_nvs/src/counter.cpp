/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "counter.h"

#include <tuple>

#include <psa/crypto.h>
#include <zephyr/logging/log.h>
#include <zephyr/settings/settings.h>

LOG_MODULE_DECLARE(external_nvs);

namespace DoorLock::ExternalNvs::Counter {

namespace {

static_assert((kUniqueIdSize % 4) == 0, "Unique ID size must be a multiple of 4");

constexpr auto kSettingsPrefix{ CONFIG_DOOR_LOCK_EXTERNAL_NVS_ROLLBACK_PROTECTION_SETTINGS_PREFIX };
constexpr auto kSettingsUniqueIdKey{ "id" };

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

int PrepareUniqueIdKey(KeyBuffer &keyBuffer)
{
	auto err = snprintf(keyBuffer.data(), keyBuffer.size(), "%s/%s", kSettingsPrefix, kSettingsUniqueIdKey);
	if (err < 0 || static_cast<size_t>(err) >= keyBuffer.size()) {
		LOG_ERR("Failed to format unique ID key");
		return -ENOMEM;
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

} // namespace

int Init()
{
	auto err = settings_subsys_init();
	if (err != 0) {
		LOG_ERR("Failed to initialize settings subsystem: %d", err);
		return err;
	}

	KeyBuffer keyBuffer;
	err = PrepareUniqueIdKey(keyBuffer);
	if (err != 0) {
		return err;
	}

	err = settings_load_one(keyBuffer.data(), sUniqueId.data(), sUniqueId.size());
	if (err != static_cast<ssize_t>(sUniqueId.size())) {
		LOG_INF("Failed to load unique ID: %d", err);

		err = GenerateUniqueId(sUniqueId);
		if (err != 0) {
			return err;
		}

		err = settings_save_one(keyBuffer.data(), sUniqueId.data(), sUniqueId.size());
		if (err != 0) {
			LOG_ERR("Failed to save unique ID: %d", err);
			return err;
		}
	}

	sInitialized = true;

	return 0;
}

int Clear()
{
	sInitialized = false;

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

	KeyBuffer keyBuffer;
	err = PrepareUniqueIdKey(keyBuffer);
	if (err != 0) {
		return err;
	}

	err = GenerateUniqueId(sUniqueId);
	if (err != 0) {
		return err;
	}

	err = settings_save_one(keyBuffer.data(), sUniqueId.data(), sUniqueId.size());
	if (err != 0) {
		LOG_ERR("Failed to save unique ID: %d", err);
		return err;
	}

	sInitialized = true;

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
