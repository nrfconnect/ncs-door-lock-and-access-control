/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "ncs_pal_isr.h"

#include <zephyr/kernel.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(pal_isr, CONFIG_NFC_LOG_LEVEL);

static nfc_isr_cb nfc_isr_callback = NULL;

static void nfc_isr_work_handler(struct k_work *work);
K_WORK_DEFINE(nfc_isr_work, nfc_isr_work_handler);

void ncs_pal_isr_cb_set(nfc_isr_cb cb)
{
	nfc_isr_callback = cb;
}

void ncs_pal_isr_trigger(void)
{
	(void)k_work_submit(&nfc_isr_work);
}

static void nfc_isr_work_handler(struct k_work *work)
{
	ARG_UNUSED(work);

	LOG_DBG("NFC ISR");
	if (nfc_isr_callback) {
		nfc_isr_callback();
	}
}
