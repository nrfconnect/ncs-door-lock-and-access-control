/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef NCS_PAL_CRITICAL_SECTION_H
#define NCS_PAL_CRITICAL_SECTION_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Lock the mutex
 */
void ncs_pal_critical_section_start(void);

/**
 * @brief Unlock the mutex
 */
void ncs_pal_critical_section_stop(void);

#ifdef __cplusplus
}
#endif

#endif /* NCS_PAL_CRITICAL_SECTION_H */
