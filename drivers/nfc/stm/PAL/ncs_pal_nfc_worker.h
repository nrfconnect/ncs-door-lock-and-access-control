/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef NCS_PAL_NFC_WORKER_H
#define NCS_PAL_NFC_WORKER_H

#include <stdbool.h>
#include <zephyr/kernel.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*thread_func_t)(void *, void *, void *);

/**
 * @brief Start NFC worker thread with CONFIG_WORKER_THREAD_PRIORITY priority.
 *
 * @param thread_func Pointer to the function that will be executed by the NFC worker thread.
 *
 * @return The thread ID when success, nullptr otherwise.
 */
k_tid_t ncs_pal_nfc_worker_start(thread_func_t thread_func);

#ifdef __cplusplus
}
#endif

#endif /* NCS_PAL_NFC_WORKER_H */
