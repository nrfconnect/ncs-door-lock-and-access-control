/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef NCS_PAL_GPIO_H
#define NCS_PAL_GPIO_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize GPIO and interrupt for GPIO.
 *
 * @return 0 when success, error code otherwise.
 */
int ncs_pal_gpio_init(void);

/**
 * @brief Check if pin has level high.
 *
 * @return TRUE when GPIO level high, false otherwise.
 */
bool ncs_pal_gpio_is_set(int port, int pin);

/**
 * @brief Set PWR pin level high on the PCA64176 shield.
 * The PWR pin is required to switch ON a X-NUCLE NFC shield.
 *
 * @return 0 when success, error code otherwise.
 */
int ncs_pal_pwr_pin_set(void);

#ifdef __cplusplus
}
#endif

#endif /* NCS_PAL_GPIO_H */
