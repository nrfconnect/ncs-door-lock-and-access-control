/*
 * SPDX-FileCopyrightText: Copyright (c) 2021 Qorvo, Inc.
 * SPDX-License-Identifier: GPL-2.0 OR Apache-2.0
 */

#pragma once

#include <stdbool.h>

#define SPI_ERR_BASE -8000

#define SPI_ERR_NOCHAN (SPI_ERR_BASE - 1)
#define SPI_ERR_INFNOTFOUND (SPI_ERR_BASE - 2)
#define SPI_ERR_NOMEM (SPI_ERR_BASE - 3)
#define SPI_ERR_READ_INCOMPLETE (SPI_ERR_BASE - 4)
#define SPI_ERR_GPIO_WRITE_CMD_INCOMPLETE (SPI_ERR_BASE - 5)
#define SPI_ERR_GPIO_READ_CMD_INCOMPLETE (SPI_ERR_BASE - 6)
#define SPI_ERR_IRQ_LINE_TIMEOUT (SPI_ERR_BASE - 7)
#define SPI_ERR_WRITE_INCOMPLETE (SPI_ERR_BASE - 8)
#define SPI_ERR_RW_INCOMPLETE (SPI_ERR_BASE - 9)
#define SPI_ERR_INVALID_STC_LEN (SPI_ERR_BASE - 10)
#define SPI_ERR_WAIT_READY_TIMEOUT (SPI_ERR_BASE - 11)

#define SPI_ERR_LIB_BASE (SPI_ERR_BASE - 500)
#define SPI_ERR_LIB(rc) (SPI_ERR_LIB_BASE - (rc))

#define SPI_RESET_DURATION_MS 2

#define QMROM_SPI_OPS_NAME_LEN 8

struct qmrom_spi_handle;

/**
 * struct qmrom_spi_ops - SPI handle operations
 */
struct qmrom_spi_ops {
	/**
	 * @name: Driver name.
	 */
	const char name[QMROM_SPI_OPS_NAME_LEN];

	/**
	 * @list_devices: List available SPI devices.
	 *
	 * **Return:** 0 on success, negative error code on failure.
	 */
	int (*list_devices)(void);

	/**
	 * @find_device: Find a SPI device by serial.
	 *
	 * **Return:** Device index on success, negative error code on failure.
	 */
	int (*find_device)(const char *serial);

	/**
	 * @init: Initialize a SPI handle.
	 *
	 * **Return:** SPI handle on success, NULL on failure.
	 */
	struct qmrom_spi_handle *(*init)(int device_index, int speed_hz);
	/**
	 * @deinit: Deinitialize a SPI handle.
	 *
	 * **Return:** 0 on success, negative error code on failure.
	 */
	int (*deinit)(struct qmrom_spi_handle *handle);

	/**
	 * @transfer: Perform a SPI transfer.
	 *
	 * **Return:** 0 on success, negative error code on failure.
	 */
	int (*transfer)(struct qmrom_spi_handle *handle, const void *tx_buf,
			void *rx_buf, unsigned int size);
	/**
	 * @set_cs_level: Set the CS level.
	 *
	 * **Return:** 0 on success, negative error code on failure.
	 */
	int (*set_cs_level)(struct qmrom_spi_handle *handle, bool level);
	/**
	 * @reset_device: Reset the device.
	 *
	 * **Return:** 0 on success, negative error code on failure.
	 */
	int (*reset_device)(struct qmrom_spi_handle *handle);
	/**
	 * @wait_for_irq_line: Wait for the IRQ line.
	 *
	 * This function is optional. If not defined, a generic implementation
	 * relying on &qmrom_spi_ops.read_irq_line is used.
	 *
	 * **Return:** 0 on success, negative error code on failure.
	 */
	int (*wait_for_irq_line)(struct qmrom_spi_handle *handle,
				 int timeout_ms);
	/**
	 * @read_irq_line: Read the IRQ line.
	 *
	 * **Return:** IRQ line state on success, negative error code on
	 * failure.
	 */
	int (*read_irq_line)(struct qmrom_spi_handle *handle);
	/**
	 * @set_speed: Set the SPI speed.
	 *
	 * **Return:** 0 on success, negative error code on failure.
	 */
	int (*set_speed)(struct qmrom_spi_handle *handle, int speed_hz);
};

struct qmrom_spi_handle {
	const struct qmrom_spi_ops *ops;
	bool ignore_irq_line;
};

int qmrom_spi_register_driver(const struct qmrom_spi_ops *ops);
void qmrom_spi_unregister_drivers(void);
int qmrom_spi_list_devices(int driver_index);
int qmrom_spi_find_driver(const char *name);
int qmrom_spi_find_device(int *driver_index, const char *serial);

struct qmrom_spi_handle *qmrom_spi_init(int driver_index, int device_index,
					int speed_hz);
int qmrom_spi_deinit(struct qmrom_spi_handle *handle);

int qmrom_spi_ignore_irq_line(struct qmrom_spi_handle *handle, bool ignore);
int qmrom_spi_transfer(struct qmrom_spi_handle *handle, const void *tx_buf,
		       void *rx_buf, unsigned int size);
int qmrom_spi_set_cs_level(struct qmrom_spi_handle *handle, bool level);
int qmrom_spi_reset_device(struct qmrom_spi_handle *handle);
int qmrom_spi_wait_for_irq_line(struct qmrom_spi_handle *handle,
				int timeout_ms);
int qmrom_spi_read_irq_line(struct qmrom_spi_handle *handle);
int qmrom_spi_set_speed(struct qmrom_spi_handle *handle, int speed_hz);
