/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/ztest.h>

#include <settings_utils/settings_utils.h>
#include <zephyr/fs/nvs.h>
#include <zephyr/fs/zms.h>
#include <zephyr/settings/settings.h>

#include <array>
#include <cstdint>

namespace {

using namespace DoorLock::SettingsUtils;

struct TestData {
	uint32_t mFirst;
	uint16_t mSecond;
	uint8_t mThird;
};

void ClearSettingsStorage()
{
	auto rc = settings_subsys_init();
	zassert_equal(rc, 0, "Failed to initialize settings subsystem");

	void *storage = nullptr;
	rc = settings_storage_get(&storage);
	zassert_equal(rc, 0, "Failed to get settings storage");

#ifdef CONFIG_SETTINGS_NVS
	rc = nvs_clear(static_cast<nvs_fs *>(storage));
	zassert_equal(rc, 0, "Failed to clear NVS settings storage");
	rc = nvs_mount(static_cast<nvs_fs *>(storage));
	zassert_equal(rc, 0, "Failed to mount NVS settings storage");
#endif // CONFIG_SETTINGS_NVS
#ifdef CONFIG_SETTINGS_ZMS
	rc = zms_clear(static_cast<zms_fs *>(storage));
	zassert_equal(rc, 0, "Failed to clear ZMS settings storage");
	rc = zms_mount(static_cast<zms_fs *>(storage));
	zassert_equal(rc, 0, "Failed to mount ZMS settings storage");
#endif // CONFIG_SETTINGS_ZMS
}

} // namespace

ZTEST(settings_utils_tests, test_save_load_round_trip_raw_bytes)
{
	constexpr const char *kKeyName = "settings_utils/raw";
	constexpr std::array<uint8_t, 6> kExpectedData{ 0x01, 0x02, 0x7f, 0x80, 0xfe, 0xff };

	auto rc = Save(kKeyName, kExpectedData.data(), kExpectedData.size());
	zassert_equal(rc, 0, "Failed to save raw bytes");

	std::array<uint8_t, kExpectedData.size()> readData{};
	size_t readSize{ readData.size() };
	rc = Load(kKeyName, readData.data(), readSize);
	zassert_equal(rc, 0, "Failed to load raw bytes");
	zassert_equal(readSize, kExpectedData.size(), "Loaded raw byte size mismatch");
	zassert_mem_equal(readData.data(), kExpectedData.data(), kExpectedData.size(), "Loaded raw data mismatch");
}

ZTEST(settings_utils_tests, test_template_save_load_round_trip)
{
	constexpr const char *kKeyName = "settings_utils/template";
	constexpr TestData kExpected{ 0x12345678, 0xbeef, 0x5a };

	auto rc = Save(kKeyName, kExpected);
	zassert_equal(rc, 0, "Failed to save data using template");

	TestData readData{};
	rc = Load(kKeyName, readData);
	zassert_equal(rc, 0, "Failed to load data using template");
	zassert_equal(readData.mFirst, kExpected.mFirst, "Loaded data first field mismatch");
	zassert_equal(readData.mSecond, kExpected.mSecond, "Loaded data second field mismatch");
	zassert_equal(readData.mThird, kExpected.mThird, "Loaded data third field mismatch");
}

ZTEST(settings_utils_tests, test_load_non_existing_key_returns_enoent)
{
	std::array<uint8_t, 8> readData{};
	size_t readSize{ readData.size() };

	const auto rc = Load("settings_utils/missing", readData.data(), readSize);
	zassert_equal(rc, -ENOENT, "Expected -ENOENT for non-existing key");
	zassert_equal(readSize, 0U, "Expected zero read size for non-existing key");
}

ZTEST(settings_utils_tests, test_load_small_buffer_returns_enomem)
{
	constexpr const char *kKeyName = "settings_utils/small_buffer";
	constexpr std::array<uint8_t, 8> kExpectedData{ 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x80 };

	auto rc = Save(kKeyName, kExpectedData.data(), kExpectedData.size());
	zassert_equal(rc, 0, "Failed to save value for small-buffer test");

	std::array<uint8_t, 2> readData{};
	size_t readSize{ readData.size() };
	rc = Load(kKeyName, readData.data(), readSize);
	zassert_equal(rc, -ENOMEM, "Expected -ENOMEM for too small read buffer");
	zassert_equal(readSize, kExpectedData.size(), "Expected required size to be returned");
}

ZTEST(settings_utils_tests, test_loadexact_size_mismatch_returns_einval)
{
	constexpr const char *kKeyName = "settings_utils/load_exact_mismatch";
	constexpr uint32_t kStoredValue{ 0xaabbccdd };

	auto rc = Save(kKeyName, kStoredValue);
	zassert_equal(rc, 0, "Failed to save value for LoadExact mismatch test");

	std::array<uint8_t, sizeof(kStoredValue) + 2> readData{};
	rc = LoadExact(kKeyName, readData.data(), readData.size());
	zassert_equal(rc, -EINVAL, "Expected -EINVAL for size mismatch in LoadExact");
}

ZTEST(settings_utils_tests, test_delete_then_load_returns_enoent)
{
	constexpr const char *kKeyName = "settings_utils/delete";
	constexpr uint32_t kValue{ 0x44332211 };

	auto rc = Save(kKeyName, kValue);
	zassert_equal(rc, 0, "Failed to save value for delete test");

	rc = Delete(kKeyName);
	zassert_equal(rc, 0, "Failed to delete stored key");

	uint32_t loadedValue{};
	size_t readSize{ sizeof(loadedValue) };
	rc = Load(kKeyName, &loadedValue, readSize);
	zassert_equal(rc, -ENOENT, "Expected -ENOENT after deleting key");
}

void *setup_suite(void)
{
	ClearSettingsStorage();
	return nullptr;
}

ZTEST_SUITE(settings_utils_tests, nullptr, setup_suite, nullptr, nullptr, nullptr);
