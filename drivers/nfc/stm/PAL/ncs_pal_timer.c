/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/kernel.h>
#include <zephyr/sys/__assert.h>

#include "ncs_pal_semaphore.h"
#include "ncs_pal_timer.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(pal_timer, CONFIG_NFC_LOG_LEVEL);

typedef struct {
	struct k_timer timer;
	bool in_use;
} ncs_pal_timer_t;

static ncs_pal_timer_t timers[CONFIG_PAL_MAX_TIMERS_NUM];

uint32_t ncs_pal_get_sys_tick()
{
	return k_uptime_get_32();
}

static void timer_expiry_callback(struct k_timer *timer)
{
	ncs_pal_give_semaphore();
}

void ncs_pal_timers_init()
{
	int timer_id = 0;
	for (timer_id = 0; timer_id < CONFIG_PAL_MAX_TIMERS_NUM; timer_id++) {
		k_timer_init(&timers[timer_id].timer, timer_expiry_callback, NULL);
		timers[timer_id].in_use = false;
	}
	LOG_DBG("%d timers initialized", timer_id + 1);
}

int ncs_pal_timer_create(uint16_t time_ms)
{
	int timer_id = 0;
	for (timer_id = 0; timer_id < CONFIG_PAL_MAX_TIMERS_NUM; timer_id++) {
		if (!timers[timer_id].in_use) {
			k_timer_start(&timers[timer_id].timer, K_MSEC(time_ms), K_NO_WAIT);
			timers[timer_id].in_use = true;
			// Timer ID must be greater than 0, so we add 1 to the ID.
			return timer_id + 1;
		}
	}
	__ASSERT(false, "No available timers, please change PAL_MAX_TIMERS_NUM config.");
	// The 0 value is reserved for RFAL (the timer is disabled).
	return 0;
}

bool ncs_pal_timer_is_expired(uint8_t timer_id)
{
	if (!IN_RANGE(timer_id, 1, CONFIG_PAL_MAX_TIMERS_NUM)) {
		LOG_DBG("Invalid timer ID %d", timer_id);
		return true;
	}

	if (k_timer_remaining_ticks(&timers[timer_id - 1].timer) == 0) {
		return true;
	}
	return false;
}

void ncs_pal_timer_destroy(uint8_t timer_id)
{
	if (!IN_RANGE(timer_id, 1, CONFIG_PAL_MAX_TIMERS_NUM)) {
		LOG_DBG("Invalid timer ID %d", timer_id);
		return;
	}

	if (timers[timer_id - 1].in_use) {
		k_timer_stop(&timers[timer_id - 1].timer);
		timers[timer_id - 1].in_use = false;
	}
}

void ncs_pal_delay(uint16_t delay_ms)
{
	k_sleep(K_MSEC(delay_ms));
}

uint32_t ncs_pal_timer_get_remaining(uint8_t timer_id)
{
	if (!IN_RANGE(timer_id, 1, CONFIG_PAL_MAX_TIMERS_NUM)) {
		LOG_DBG("Invalid timer ID %d", timer_id);
		return 0;
	}
	k_ticks_t ticks = k_timer_remaining_ticks(&timers[timer_id - 1].timer);
	return k_ticks_to_ms_floor32(ticks);
}
