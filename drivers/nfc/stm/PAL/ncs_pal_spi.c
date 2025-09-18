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

/* Timing defined by spec. */
#define T_NCS_SCLK 1

/* SPI hardware configuration. */
static const struct spi_config spi_cfg = {
	.frequency = DT_PROP(DT_INST(0, x_nucleo_nfc), spi_max_frequency),
	.operation = ((IS_ENABLED(CONFIG_RFAL_SINGLE_SPI_BUS) ? SPI_LOCK_ON : 0) | SPI_OP_MODE_MASTER |
		      SPI_WORD_SET(8) | SPI_TRANSFER_MSB | SPI_LINES_SINGLE | SPI_MODE_CPHA),
	.slave = DT_REG_ADDR(DT_INST(0, x_nucleo_nfc)),
	.cs = { .gpio = SPI_CS_GPIOS_DT_SPEC_GET(DT_INST(0, x_nucleo_nfc)), .delay = T_NCS_SCLK }
};

static inline int transceive(const struct device *dev, const struct spi_config *config, const struct spi_buf_set *tx,
			     const struct spi_buf_set *rx)
{
	int err = spi_transceive(dev, config, tx, rx);
	if (IS_ENABLED(CONFIG_RFAL_SINGLE_SPI_BUS)) {
		// Try to release the SPI device always, even if the transfer failed.
		err = spi_release(dev, config);
		if (err < 0) {
			LOG_ERR("SPI release failed, err: %d.", err);
		}
	}
	return err;
}

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

	err = transceive(spi_dev, &spi_cfg, (txData ? &tx : NULL), (rxData ? &rx : NULL));
	if (err) {
		LOG_ERR("SPI transceive failed, err: %d.", err);
		return err;
	}

	return 0;
}
