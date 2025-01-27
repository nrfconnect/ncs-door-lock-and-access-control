/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/kernel.h>

#include "ncs_pal_timer.h"

uint32_t ncs_pal_get_sys_tick()
{
	// FIXME: This is only lower 32-bit value of system uptime.
	return k_uptime_get_32();
}

uint32_t ncs_pal_timer_create(uint16_t time)
{
	return (k_uptime_get_32() + time);
}

bool ncs_pal_timer_is_expired(uint32_t timer)
{
	return (k_uptime_get_32() >= timer);
}

void ncs_pal_delay(uint16_t tOut)
{
	k_sleep(K_MSEC(tOut));
}
