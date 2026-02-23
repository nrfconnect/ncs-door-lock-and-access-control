/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef NCS_PAL_NFC_WORKER_H
#define NCS_PAL_NFC_WORKER_H

#include <zephyr/kernel.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Submit a work item to the dedicated Aliro workqueue.

 * @note Must be implemented by the application.
 *
 */
void ncs_pal_submit_nfc_work();

#ifdef __cplusplus
}
#endif

#endif /* NCS_PAL_NFC_WORKER_H */
