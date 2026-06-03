/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/drivers/spi.h>
#include <zephyr/logging/log.h>

#include "ncs_pal_spi.h"

LOG_MODULE_REGISTER(pal_spi, CONFIG_NFC_LOG_LEVEL);

static const struct device *spi_dev = DEVICE_DT_GET(DT_BUS(DT_INST(0, x_nucleo_nfc)));

/* SPI hardware configuration. */
#define SPI_OPERATION (SPI_OP_MODE_MASTER | SPI_WORD_SET(8) | SPI_TRANSFER_MSB | SPI_LINES_SINGLE | SPI_MODE_CPHA)

static const struct spi_config spi_cfg = SPI_CONFIG_DT(DT_INST(0, x_nucleo_nfc), SPI_OPERATION);

int ncs_pal_spi_init(void)
{
	LOG_INF("Initializing SPI device");

	if (!device_is_ready(spi_cfg.cs.gpio.port)) {
		LOG_ERR("GPIO device %s is not ready!", spi_cfg.cs.gpio.port->name);

		return -ENXIO;
	}

	if (!device_is_ready(spi_dev)) {
		LOG_ERR("SPI device %s is not ready!", spi_dev->name);
		return -ENXIO;
	}

	return 0;
}

int ncs_pal_spi_transfer(const uint8_t *txData, uint8_t *rxData, uint16_t length)
{
	int err;

	if (!device_is_ready(spi_dev)) {
		LOG_ERR("SPI device %s is not ready!", spi_dev->name);
		return -ENXIO;
	}

	if (!txData && !rxData) {
		return -EINVAL;
	}

	if (!length) {
		return 0;
	}

	const struct spi_buf tx_bufs[] = { { .buf = (uint8_t *)txData, .len = length } };
	const struct spi_buf_set tx = { .buffers = tx_bufs, .count = ARRAY_SIZE(tx_bufs) };

	const struct spi_buf rx_bufs[] = { { .buf = rxData, .len = length } };
	const struct spi_buf_set rx = { .buffers = rx_bufs, .count = ARRAY_SIZE(rx_bufs) };

	err = spi_transceive(spi_dev, &spi_cfg, (txData ? &tx : NULL), (rxData ? &rx : NULL));
	if (err) {
		LOG_ERR("SPI transceive failed, err: %d.", err);
		return err;
	}

	return 0;
}
