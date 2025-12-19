/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <cstdlib>

#include "aliro/utils.h"
#include "uwb_dfu.h"

#include <pm_config.h>
#include <zephyr/dfu/mcuboot.h>
#include <zephyr/logging/log.h>
#include <zephyr/storage/flash_map.h>

extern "C" {
#include <qmrom.h>
#include <qmrom_hsspi.h>
#include <qmrom_spi.h>
}

LOG_MODULE_DECLARE(UwbImpl);

extern "C" const struct qmrom_spi_ops zephyr_spi_ops;

namespace Aliro::Uwb::Dfu {

namespace {

constexpr size_t kMaxChunkSize = 2000;

constexpr const char *kQm35VersionFormat = "%hhu.%hhu.%hurc%u_%*s";
constexpr int kQm35VersionElements = 4;

constexpr int kQm35DfuStatusSpiInitError = 1;
constexpr int kQm35DfuStatusHsspiInitError = 2;
constexpr int kQm35DfuStatusQmromInitError = 3;

} // namespace

static uint64_t VersionToUint64(mcuboot_img_sem_ver *version)
{
	uint64_t result = 0;

	result |= (static_cast<uint64_t>(version->major) << 56);
	result |= (static_cast<uint64_t>(version->minor) << 48);
	result |= (static_cast<uint64_t>(version->revision) << 32);
	result |= static_cast<uint64_t>(version->build_num);

	return result;
}

static bool ParseVersionString(const char *str, mcuboot_img_sem_ver *version)
{
	return sscanf(str, kQm35VersionFormat, &version->major, &version->minor, &version->revision,
		      &version->build_num) == kQm35VersionElements;
}

bool ShouldUpdate(const char *currentVersionString)
{
	int err;
	mcuboot_img_header header;
	mcuboot_img_sem_ver currentVersion;

	err = boot_read_bank_header(QM35_DFU_IMAGE_PARTITION_ID, &header, sizeof(mcuboot_img_header));
	VerifyOrReturnFalse(!err, LOG_ERR("Error when reading QM35 FW primary slot: %d", err));

	VerifyOrReturnFalse(ParseVersionString(currentVersionString, &currentVersion),
			    LOG_ERR("Failed to parse current version string"));

	if (IS_ENABLED(CONFIG_DOOR_LOCK_UWB_QM35_DFU_VERSION_COMPARISON_DIFFERENT)) {
		return VersionToUint64(&header.h.v1.sem_ver) != VersionToUint64(&currentVersion);
	} else {
		return VersionToUint64(&header.h.v1.sem_ver) > VersionToUint64(&currentVersion);
	}
}

int PerformFirmwareUpdate()
{
	int ret;
	struct qmrom_spi_handle *spi_handle = NULL;
	struct qmrom_hsspi_handle *hsspi_handle = NULL;
	struct qmrom_handle *handle = NULL;
	const struct flash_area *fap;
	uint32_t offset = 0;
	mcuboot_img_header header;
	uint8_t firmwareChunkBuffer[kMaxChunkSize];

	ret = boot_read_bank_header(QM35_DFU_IMAGE_PARTITION_ID, &header, sizeof(mcuboot_img_header));
	VerifyOrReturnValue(ret == 0, ret, LOG_WRN("Error when reading QM35 FW primary slot: %d", ret));

	ret = qmrom_spi_register_driver(&zephyr_spi_ops);
	VerifyOrReturnValue(ret == 0, ret, LOG_ERR("Failed to register zephyr SPI driver: %d", ret));

	spi_handle = qmrom_spi_init(0, 0, 0);
	VerifyOrExit(spi_handle, {
		ret = kQm35DfuStatusSpiInitError;
		LOG_ERR("Couldn't initialize SPI interface");
	});

	hsspi_handle = qmrom_hsspi_init(QMROM_HSSPI_SPI, spi_handle);
	VerifyOrExit(hsspi_handle, {
		ret = kQm35DfuStatusHsspiInitError;
		LOG_ERR("Couldn't initialize HSSPI interface");
	});

	handle = qmrom_init(hsspi_handle);
	VerifyOrExit(handle, {
		ret = kQm35DfuStatusQmromInitError;
		LOG_ERR("Couldn't initialize QMROM handle");
	});

	ret = qmrom_enter_chunk_mode(handle);
	VerifyOrExit(ret == 0, LOG_ERR("Entering chunk mode failed with error %d", ret));

	ret = flash_area_open(QM35_DFU_FIRMWARE_PARTITION_ID, &fap);
	VerifyOrExit(ret == 0, LOG_ERR("Failed to open flash area, error: %d", ret));

	while (offset < header.h.v1.image_size) {
		uint32_t to_send = kMaxChunkSize;

		if (offset + to_send > header.h.v1.image_size) {
			to_send = header.h.v1.image_size - offset;
		}

		ret = flash_area_read(fap, offset, firmwareChunkBuffer, to_send);
		VerifyOrExit(ret == 0, LOG_ERR("Failed to read firmware from flash, error: %d", ret));

		ret = qmrom_write_chunk(handle, reinterpret_cast<const char *>(firmwareChunkBuffer), to_send);
		VerifyOrExit(ret == 0, LOG_ERR("Writing chunk at offset %d failed with error %d", offset, ret));

		offset += to_send;
	}

	flash_area_close(fap);

	LOG_INF("Chunked flashing successful");

	ret = qmrom_exit_chunk_mode(handle);
	VerifyOrExit(ret == 0, LOG_ERR("Exiting chunk mode failed with error %d", ret));

	ret = qmrom_reset_device(handle);
	VerifyOrExit(ret == 0, LOG_ERR("Resetting device failed with error %d", ret));

exit:
	if (handle) {
		qmrom_deinit(handle);
	}

	if (hsspi_handle) {
		qmrom_hsspi_deinit(hsspi_handle);
	}

	if (spi_handle) {
		qmrom_spi_deinit(spi_handle);
	}

	qmrom_spi_unregister_drivers();

	return ret;
}

} // namespace Aliro::Uwb::Dfu
