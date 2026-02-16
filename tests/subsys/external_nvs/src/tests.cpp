/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/ztest.h>

#include <algorithm>
#include <array>
#include <cstring>

#include <external_nvs/external_nvs.h>
#include <psa/crypto.h>
#include <zephyr/fs/nvs.h>
#include <zephyr/fs/zms.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/settings/settings.h>
#include <zephyr/storage/flash_map.h>

LOG_MODULE_REGISTER(external_nvs_tests);

using namespace DoorLock;

namespace {

#if CONFIG_PARTITION_MANAGER_ENABLED
constexpr uint8_t kExternalNvsPartitionId{ FIXED_PARTITION_ID(external_nvs) };
#else
constexpr uint8_t kExternalNvsPartitionId{ FIXED_PARTITION_ID(external_nvs_partition) };
#endif // CONFIG_PARTITION_MANAGER_ENABLED

using Buffer = std::array<uint8_t, CONFIG_DOOR_LOCK_EXTERNAL_NVS_MAX_DATA_SIZE>;

void ClearStoragePartition()
{
	const flash_area *fa;
	auto rc = flash_area_open(kExternalNvsPartitionId, &fa);
	zassert_equal(rc, 0, "Failed to open flash area");

	rc = flash_area_flatten(fa, 0, fa->fa_size);
	zassert_equal(rc, 0, "Failed to flatten flash area");

	flash_area_close(fa);
}

void ClearSettings()
{
	auto rc = settings_subsys_init();
	zassert_equal(rc, 0, "Failed to initialize settings subsystem");

	void *storage;
	rc = settings_storage_get(&storage);
	zassert_equal(rc, 0, "Failed to get storage");

#ifdef CONFIG_SETTINGS_NVS
	rc = nvs_clear(static_cast<nvs_fs *>(storage));
	zassert_equal(rc, 0, "Failed to clear storage");
	rc = nvs_mount(static_cast<nvs_fs *>(storage));
	zassert_equal(rc, 0, "Failed to mount storage");
#endif // CONFIG_SETTINGS_NVS
#ifdef CONFIG_SETTINGS_ZMS
	rc = zms_clear(static_cast<zms_fs *>(storage));
	zassert_equal(rc, 0, "Failed to clear storage");
	rc = zms_mount(static_cast<zms_fs *>(storage));
	zassert_equal(rc, 0, "Failed to mount storage");
#endif // CONFIG_SETTINGS_ZMS
}

void DumpSettings()
{
	const auto cb = [](const char *key, size_t len, settings_read_cb read_cb, void *cb_arg, void *param) {
		ARG_UNUSED(param);

		std::array<uint8_t, 128> buffer;
		const auto rc = read_cb(cb_arg, buffer.data(), std::min(len, buffer.size()));

		if (rc < 0) {
			LOG_ERR("Failed to read key: %s", key);
		} else if (rc == 0) {
			LOG_ERR("Key deleted: %s", key);
		} else {
			LOG_HEXDUMP_INF(buffer.data(), static_cast<size_t>(rc), key);
		}

		return 0;
	};

	LOG_INF("\nSettings:");

	const auto err = settings_load_subtree_direct(nullptr, cb, nullptr);
	if (err) {
		LOG_ERR("Failed to load settings: %d", err);
	}
}

} // namespace

ZTEST(external_nvs_tests, test_read_non_existent_id)
{
	constexpr ExternalNvs::Id id{ 0 };

	Buffer buffer;
	size_t readLength{ buffer.size() };
	auto rc = ExternalNvs::Read(id, buffer.data(), readLength);
	zassert_equal(rc, -ENOENT, "Expected error code -ENOENT");
}

ZTEST(external_nvs_tests, test_write_read_max_data_size)
{
	constexpr ExternalNvs::Id id{ 1 };

	Buffer writeBuffer;
	const auto status = psa_generate_random(writeBuffer.data(), writeBuffer.size());
	zassert_equal(status, PSA_SUCCESS, "Failed to generate random data");

	auto rc = ExternalNvs::Write(id, writeBuffer.data(), writeBuffer.size());
	zassert_equal(rc, 0, "Failed to write data");

	Buffer readBuffer;
	size_t readLength{ readBuffer.size() };
	rc = ExternalNvs::Read(id, readBuffer.data(), readLength);
	zassert_equal(rc, 0, "Failed to read data");
	zassert_equal(readLength, readBuffer.size(), "Read length mismatch");
	zassert_equal(readBuffer, writeBuffer, "Data mismatch");
}

ZTEST(external_nvs_tests, test_write_read_hello)
{
	constexpr ExternalNvs::Id id{ 2 };

	const char *hello = "Hello External NVM!";
	const size_t helloLength = strlen(hello) + 1;

	auto rc = ExternalNvs::Write(id, hello, helloLength);
	zassert_equal(rc, 0, "Failed to write data");

	Buffer readBuffer;
	size_t readLength{ readBuffer.size() };
	rc = ExternalNvs::Read(id, readBuffer.data(), readLength);
	zassert_equal(rc, 0, "Failed to read data");
	zassert_equal(readLength, helloLength, "Read length mismatch");
	zassert_mem_equal(hello, readBuffer.data(), helloLength, "Data mismatch");
}

void *setup_suite(void)
{
	ClearStoragePartition();
	ClearSettings();

	auto rc = ::DoorLock::ExternalNvs::Init(kExternalNvsPartitionId);
	zassert_equal(rc, 0, "Failed to initialize External NVS");

	return nullptr;
}

void teardown_suite(void *)
{
	DumpSettings();
}

ZTEST_SUITE(external_nvs_tests, nullptr, setup_suite, nullptr, nullptr, teardown_suite);
