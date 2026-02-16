/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "storage.h"

#include <stdint.h>
#include <zephyr/fs/nvs.h>
#include <zephyr/logging/log.h>
#include <zephyr/storage/flash_map.h>

LOG_MODULE_DECLARE(external_nvs);

namespace DoorLock::ExternalNvs::Storage {

namespace {

nvs_fs nvs;

} // namespace

int Init(uint8_t partitionId)
{
	const flash_area *fa;

	auto rc = flash_area_open(partitionId, &fa);
	if (rc) {
		LOG_ERR("flash_area_open failed: %d", rc);
		return rc;
	}
	LOG_DBG("flash_area_open successful, offset: 0x%lx, size: 0x%x, device: %s", fa->fa_off, fa->fa_size,
		fa->fa_dev->name);

	flash_sector hw_flash_sector;
	uint32_t sector_cnt{ 1 };

	rc = flash_area_get_sectors(partitionId, &sector_cnt, &hw_flash_sector);
	if (rc != 0 && rc != -ENOMEM) {
		LOG_ERR("flash_area_get_sectors failed: %d", rc);
		return rc;
	}

	sector_cnt = fa->fa_size / hw_flash_sector.fs_size;
	LOG_DBG("flash_area_get_sectors successful, sector_size: 0x%x, sector_count: %u", hw_flash_sector.fs_size,
		sector_cnt);

	nvs.offset = fa->fa_off;
	nvs.sector_size = hw_flash_sector.fs_size;
	nvs.sector_count = static_cast<uint16_t>(sector_cnt);
	nvs.flash_device = fa->fa_dev;

	rc = nvs_mount(&nvs);
	if (rc) {
		LOG_ERR("nvs_mount failed: %d", rc);
		return rc;
	}

	return 0;
}

int Clear()
{
	auto rc = nvs_clear(&nvs);
	if (rc) {
		LOG_ERR("nvs_clear failed: %d", rc);
		return rc;
	}

	rc = nvs_mount(&nvs);
	if (rc) {
		LOG_ERR("nvs_mount failed: %d", rc);
		return rc;
	}

	return 0;
}

int Write(Id id, const void *data, size_t len)
{
	const auto rc = nvs_write(&nvs, id, data, len);
	if (rc < 0) {
		LOG_ERR("nvs_write failed: %d", rc);
		return rc;
	} else if (rc > 0 && static_cast<size_t>(rc) != len) {
		LOG_ERR("nvs_write failed to write %zu bytes: %d", len, rc);
		return -EIO;
	}

	return 0;
}

int Read(Id id, void *data, size_t &len)
{
	const auto rc = nvs_read(&nvs, id, data, len);
	if (rc < 0) {
		LOG_ERR("nvs_read failed: %d", rc);
		return rc;
	}

	len = rc;
	return 0;
}

int Delete(Id id)
{
	const auto rc = nvs_delete(&nvs, id);
	if (rc) {
		LOG_ERR("nvs_delete failed: %d", rc);
		return rc;
	}

	return 0;
}

} // namespace DoorLock::ExternalNvs::Storage
