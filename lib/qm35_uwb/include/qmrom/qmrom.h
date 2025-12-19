/*
 * SPDX-FileCopyrightText: Copyright (c) 2021 Qorvo, Inc.
 * SPDX-License-Identifier: GPL-2.0 OR Apache-2.0
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>

#define QMROM_SOC_ID_LEN 32
#define QMROM_UUID_LEN 16
#define QMROM_GIT_HASH_LEN 8

#ifdef HAVE_QM357XX
#define DEVICE_VER_QM357XX_C0 0x0430
#endif
#ifdef HAVE_QM358XX
#define DEVICE_VER_QM358XX_A0 0x0440
#define DEVICE_VER_QPF51XX_A0 0x0450
#endif

struct qmrom_handle;
struct qmrom_hsspi_handle;

struct qmrom_soc_info {
	uint16_t device_id;
	uint8_t chip_revision;

	union {
#ifdef HAVE_QM357XX
		struct {
			uint8_t soc_id[QMROM_SOC_ID_LEN];
			uint8_t uuid[QMROM_UUID_LEN];
			uint32_t lcs_state;
		} qm357xx_info;
#endif
#ifdef HAVE_QM358XX
		struct {
			uint8_t uuid[QMROM_UUID_LEN];
			uint32_t customer_id;
			uint32_t product_id;
			uint32_t arb_ver_icv;
			uint32_t arb_ver_oem;
			uint8_t lcs_state;
			uint8_t working_model;
			uint8_t business_ctx;
			uint8_t secure_debug;
			uint8_t secure_boot_oem;
			uint8_t fw_encryption_oem;
			uint8_t lock_debug;
			uint8_t sec_ver_icv;
			uint8_t sec_ver_oem;
			uint8_t enc_l2_lock;
			uint8_t git_hash[QMROM_GIT_HASH_LEN];
		} qm358xx_info;
#endif
	};
};

struct firmware {
	unsigned int size;
	const char *data;
};

struct qmrom_handle *qmrom_init(struct qmrom_hsspi_handle *hsspi_handle);
int qmrom_deinit(struct qmrom_handle *handle);

struct qmrom_soc_info *qmrom_get_device_info(struct qmrom_handle *handle);
int qmrom_reset_device(struct qmrom_handle *handle);

int qmrom_flash_firmware(struct qmrom_handle *handle,
			 const struct firmware *fw);
int qmrom_flash_debug_cert(struct qmrom_handle *handle,
			   const struct firmware *fw);
int qmrom_erase_debug_cert(struct qmrom_handle *handle);

int qmrom_enter_chunk_mode(struct qmrom_handle *handle);
int qmrom_exit_chunk_mode(struct qmrom_handle *handle);
bool qmrom_is_chunk_mode_active(struct qmrom_handle *handle);
int qmrom_write_chunk(struct qmrom_handle *handle, const char *data,
		      unsigned int size);

int qmrom_generate_secrets(struct qmrom_handle *handle);
int qmrom_load_asset_package(struct qmrom_handle *handle,
			     const struct firmware *fw);
int qmrom_load_sram_firmware(struct qmrom_handle *handle,
			     const struct firmware *fw);
int qmrom_run_test_mode(struct qmrom_handle *handle);
