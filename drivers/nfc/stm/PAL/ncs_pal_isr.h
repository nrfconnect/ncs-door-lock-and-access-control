/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef NCS_PAL_ISR_H
#define NCS_PAL_ISR_H

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*nfc_isr_cb)(void);
void ncs_pal_isr_cb_set(nfc_isr_cb cb);

#ifdef __cplusplus
}
#endif

#endif /* NCS_PAL_ISR_H */
