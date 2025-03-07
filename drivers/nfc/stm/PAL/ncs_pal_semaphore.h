/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef NCS_PAL_SEMAPHORE_H
#define NCS_PAL_SEMAPHORE_H

#include <stdbool.h>
#include <zephyr/kernel.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Take the irq semaphore
 *
 * @param timeout Maximum time in ms to wait for the semaphore to become available.
 *
 */
void ncs_pal_take_semaphore(k_timeout_t timeout_ms);

/**
 * @brief Give the irq semaphore
 */
void ncs_pal_give_semaphore(void);

#ifdef __cplusplus
}
#endif

#endif /* NCS_PAL_SEMAPHORE_H */
