/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "ncs_pal_critical_section.h"
#include <zephyr/kernel.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(pal_critical_section, CONFIG_NFC_LOG_LEVEL);

K_MUTEX_DEFINE(channel_protect_mutex);

void ncs_pal_critical_section_start(void)
{
	int ret = k_mutex_lock(&channel_protect_mutex, K_FOREVER);
	if (ret) {
		LOG_ERR("Comm protection error: %d", ret);
	}
}

void ncs_pal_critical_section_stop(void)
{
	int ret = k_mutex_unlock(&channel_protect_mutex);
	if (ret) {
		LOG_ERR("Comm unprotection error: %d", ret);
	}
}
