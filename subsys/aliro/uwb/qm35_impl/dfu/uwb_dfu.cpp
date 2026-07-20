/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <cstdlib>
#include <cstring>

#include "uwb_dfu.h"
#include <doorlock/utils/utils.h>

#include <bootutil/bootutil_public.h>
#include <bootutil/image.h>

#include <zephyr/devicetree.h>
#include <zephyr/devicetree/partitions.h>
#include <zephyr/dfu/mcuboot.h>
#include <zephyr/logging/log.h>
#include <zephyr/storage/flash_map.h>

extern "C" {
#include <qmrom.h>
#include <qmrom_hsspi.h>
#include <qmrom_spi.h>
}

LOG_MODULE_DECLARE(UwbImpl, CONFIG_DOOR_LOCK_ALIRO_UWB_LOG_LEVEL);

extern "C" const struct qmrom_spi_ops zephyr_spi_ops;

namespace Aliro::Uwb::Dfu {

namespace {

#if !DT_HAS_CHOSEN(qorvo_qm35_fw_partition)
#error "UWB firmware partition not configured (set chosen qorvo,qm35-fw-partition)"
#endif

#define UWB_FW_PARTITION_NODE DT_CHOSEN(qorvo_qm35_fw_partition)

BUILD_ASSERT(DT_PARTITION_EXISTS(UWB_FW_PARTITION_NODE), "UWB firmware partition must be a fixed or mapped partition");

constexpr uint8_t kUwbFwPartitionId{ DT_PARTITION_ID(UWB_FW_PARTITION_NODE) };

constexpr size_t kMaxChunkSize = 2000;

constexpr const char *kQm35VersionPrefixFormat = "%hhu.%hhu.%hu";
constexpr int kQm35VersionPrefixElements = 3;

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
	if (sscanf(str, kQm35VersionPrefixFormat, &version->major, &version->minor, &version->revision) !=
	    kQm35VersionPrefixElements) {
		return false;
	}

	const char *buildIdStart = strrchr(str, '_');
	VerifyOrReturnFalse(buildIdStart, LOG_ERR("Failed to find '_' in version string"));

	++buildIdStart;
	VerifyOrReturnFalse(*buildIdStart != '\0', LOG_ERR("Build ID is not present in version string"));

	char *end;
	unsigned long long buildId = strtoull(buildIdStart, &end, 10);
	VerifyOrReturnFalse(end != buildIdStart, LOG_ERR("Failed to parse Build ID"));

	version->build_num = static_cast<uint32_t>(buildId);
	return true;
}

bool ShouldUpdate(const char *currentVersionString)
{
	int err;
	mcuboot_img_header header;
	mcuboot_img_sem_ver currentVersion;

	err = boot_read_bank_header(kUwbFwPartitionId, &header, sizeof(mcuboot_img_header));
	VerifyOrReturnFalse(!err, LOG_ERR("Error when reading QM35 FW primary slot: %d", err));

	VerifyOrReturnFalse(ParseVersionString(currentVersionString, &currentVersion),
			    LOG_ERR("Failed to parse current version string"));

	if (IS_ENABLED(CONFIG_DOOR_LOCK_ALIRO_UWB_QM35_DFU_VERSION_COMPARISON_DIFFERENT)) {
		return VersionToUint64(&header.h.v1.sem_ver) != VersionToUint64(&currentVersion);
	} else {
		/* build_num is a truncated build identifier with no guaranteed ordering;
		 * use CONFIG_DOOR_LOCK_ALIRO_UWB_QM35_DFU_VERSION_COMPARISON_DIFFERENT
		 * when deploying same-rc firmware where identity matters, not ordering. */
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
	uint32_t read_offset = 0;
	uint32_t write_offset = 0;
	struct image_header hdr;
	uint8_t firmwareChunkBuffer[kMaxChunkSize];

	LOG_INF("Starting firmware update");

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

	ret = flash_area_open(kUwbFwPartitionId, &fap);
	VerifyOrExit(ret == 0, LOG_ERR("Failed to open flash area, error: %d", ret));

	ret = boot_image_load_header(fap, &hdr);
	VerifyOrExit(ret == 0, LOG_ERR("Failed to load QM35 FW image header, error: %d", ret));

	read_offset = hdr.ih_hdr_size;

	while (write_offset < hdr.ih_img_size) {
		uint32_t to_send = kMaxChunkSize;

		if (write_offset + to_send > hdr.ih_img_size) {
			to_send = hdr.ih_img_size - write_offset;
		}

		ret = flash_area_read(fap, read_offset, firmwareChunkBuffer, to_send);
		VerifyOrExit(ret == 0, LOG_ERR("Failed to read firmware from flash, error: %d", ret));

		ret = qmrom_write_chunk(handle, reinterpret_cast<const char *>(firmwareChunkBuffer), to_send);
		VerifyOrExit(ret == 0, LOG_ERR("Writing chunk at offset %u failed with error %d", write_offset, ret));

		read_offset += to_send;
		write_offset += to_send;
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

	LOG_INF("Firmware update %s", (ret ? "failed" : "successful"));

	return ret;
}

} // namespace Aliro::Uwb::Dfu
