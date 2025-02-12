/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef RFAL_NCS_PAL_H
#define RFAL_NCS_PAL_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initializes the NCS PAL for RFAL.
 *
 * @return 0 when success, error code otherwise.
 */
int rfal_ncs_pal_init(void);

#ifdef __cplusplus
}
#endif

#endif /* RFAL_NCS_PAL_H */
