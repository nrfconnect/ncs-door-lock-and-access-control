/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "ncs_pal_isr.h"

#include <zephyr/kernel.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(pal_isr, CONFIG_NFC_LOG_LEVEL);

// TODO: Move to Kconfig
#define ISR_THREAD_STACK_SIZE 1024
#define ISR_THREAD_PRIORITY 0

extern struct k_sem irq_sem;

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
		k_sem_take(&irq_sem, K_FOREVER);

		LOG_DBG("NFC ISR");

		if (nfc_isr_callback) {
			nfc_isr_callback();
		}
	}
}

K_THREAD_DEFINE(nfc_isr_poll_thread, ISR_THREAD_STACK_SIZE, nfc_isr_poll_fn, NULL, NULL, NULL, ISR_THREAD_PRIORITY, 0,
		0);
