/*
 * SPDX-FileCopyrightText: Copyright (c) 2021 Qorvo, Inc.
 * SPDX-License-Identifier: GPL-2.0 OR Apache-2.0
 */

#pragma once

#include <stdbool.h>

#define HSSPI_DEFAULT_POLL_TIMEOUT_MS 500
#define HSSPI_RETRY_COUNT 3
#define HSSPI_RETRY_DELAY_MS 2

/**
 * enum qmrom_hsspi_type - HSSPI implementations.
 *
 * Used to select the HSSPI implementation to use.
 */
enum qmrom_hsspi_type {
#ifdef HAVE_HSSPI_SPI
	/**
	 * @QMROM_HSSPI_SPI: Use the ``qmrom_spi``-based implementation.
	 */
	QMROM_HSSPI_SPI,
#endif
#ifdef HAVE_HSSPI_UCI
	/**
	 * @QMROM_HSSPI_UCI: Use the ``qm-utils``-based implementation.
	 */
	QMROM_HSSPI_UCI,
#endif
	/**
	 * @QMROM_HSSPI_MAX: Number of HSSPI implementations.
	 */
	QMROM_HSSPI_MAX,
};

struct qmrom_hsspi_handle;
struct qmrom_spi_handle;

struct qmrom_hsspi_handle *qmrom_hsspi_init(enum qmrom_hsspi_type type,
					    void *init_data);
int qmrom_hsspi_deinit(struct qmrom_hsspi_handle *handle);

int qmrom_hsspi_set_poll_timeout(struct qmrom_hsspi_handle *handle,
				 int timeout_ms);
int qmrom_hsspi_get_poll_timeout(struct qmrom_hsspi_handle *handle);
int qmrom_hsspi_poll(struct qmrom_hsspi_handle *handle);
int qmrom_hsspi_read(struct qmrom_hsspi_handle *handle, void *buf,
		     unsigned int size);
int qmrom_hsspi_write(struct qmrom_hsspi_handle *handle, const void *buf,
		      unsigned int size);
int qmrom_hsspi_reset(struct qmrom_hsspi_handle *handle, bool bootrom);

/* The following functions may only be used to communicate with the QM357xx
 * firmware updater, as it does not comply with the HSSPI STC protocol. */
int qmrom_hsspi_disable_irq(struct qmrom_hsspi_handle *handle);
int qmrom_hsspi_enable_irq(struct qmrom_hsspi_handle *handle);
int qmrom_hsspi_wait_for_irq_line(struct qmrom_hsspi_handle *handle);
int qmrom_hsspi_raw_transfer(struct qmrom_hsspi_handle *handle, void *tx_buf,
			     void *rx_buf, unsigned int size);
