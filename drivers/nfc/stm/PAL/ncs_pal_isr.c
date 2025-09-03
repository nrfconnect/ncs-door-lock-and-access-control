/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "ncs_pal_isr.h"
#include "ncs_pal_semaphore.h"

#include <zephyr/kernel.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(pal_isr, CONFIG_NFC_LOG_LEVEL);

static nfc_isr_cb nfc_isr_callback = NULL;

void ncs_pal_isr_cb_set(nfc_isr_cb cb)
{
	nfc_isr_callback = cb;
}

static void nfc_isr_poll_fn(void *unused1, void *unused2, void *unused3)
{
	ARG_UNUSED(unused1);
	ARG_UNUSED(unused2);
	ARG_UNUSED(unused3);

	while (true) {
		ncs_pal_take_semaphore(K_FOREVER);
		LOG_DBG("NFC ISR");

		if (nfc_isr_callback) {
			nfc_isr_callback();
		}
	}
}

K_THREAD_DEFINE(nfc_isr_poll_thread, CONFIG_RFAL_ISR_THREAD_STACK_SIZE, nfc_isr_poll_fn, NULL, NULL, NULL,
		CONFIG_RFAL_ISR_THREAD_PRIORITY, 0, 0);
