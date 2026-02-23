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

/** @brief Set ST25R IRQ callback executed from NFC ISR worker context.
 *
 * @param cb Callback invoked on NFC IRQ.
 */
void ncs_pal_isr_cb_set(nfc_isr_cb cb);

/**
 * @brief Trigger NFC ISR worker execution (submits work item to system
 * workqueue).
 */
void ncs_pal_isr_trigger(void);

#ifdef __cplusplus
}
#endif

#endif /* NCS_PAL_ISR_H */
