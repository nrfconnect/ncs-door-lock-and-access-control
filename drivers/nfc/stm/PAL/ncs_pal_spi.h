/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef NCS_PAL_SPI_H_
#define NCS_PAL_SPI_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize SPI bus.
 *
 * @return The value 0 when success, error code otherwise.
 */
int ncs_pal_spi_init(void);

/**
 * @brief Transfer data over SPI bus.
 *
 * @param txData [input] The buffer with data to send.
 * @param rxData [input/output] Pointer to buffer where write received data.
 * @param length [input] The number of bytes to send or size of the rxData buffer.
 *
 * @return The value 0 when success, error code otherwise.
 */
int ncs_pal_spi_transfer(const uint8_t *txData, uint8_t *rxData, uint16_t length);

#ifdef __cplusplus
}
#endif

#endif /* NCS_PAL_SPI_H_ */
