/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef NCS_PAL_TIMER_H
#define NCS_PAL_TIMER_H

#ifdef __cplusplus
extern "C" {
#endif

#define timerIsRunning(t) (!ncs_pal_timer_is_expired(t))

/**
 * @brief Get system ticks.
 *
 * @return System uptime.
 */
uint32_t ncs_pal_get_sys_tick();

/**
 * @brief Get sum of system ticks with the `time` value.
 *
 * @return Sum of system ticks with the `time` value.
 */
uint32_t ncs_pal_timer_create(uint16_t time);

/**
 * @brief Verify if time elapsed.
 *
 * @return True when time elapsed, false otherwise.
 */
bool ncs_pal_timer_is_expired(uint32_t timer);

/**
 * @brief Set delay.
 */
void ncs_pal_delay(uint16_t time);

#ifdef __cplusplus
}
#endif

#endif /* NCS_PAL_TIMER_H */
