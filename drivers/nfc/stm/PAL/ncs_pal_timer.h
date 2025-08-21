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

/**
 * @brief Initializes the PAL timers.
 */
void ncs_pal_timers_init();

/**
 * @brief Get current system uptime, in milliseconds.
 *
 * @return System uptime in miliseconds.
 */
uint32_t ncs_pal_get_sys_tick();

/**
 * @brief Starts a timer with the specified time duration.
 *
 * @param time_ms the time duration in milliseconds.
 *
 * @return Timer identifier (greather than 0) or 0 if no timer is available.
 */
int ncs_pal_timer_create(uint16_t time_ms);

/**
 * @brief Stops timer with given identifier.
 *
 * @param timer_id the timer identifier.
 */
void ncs_pal_timer_destroy(uint8_t timer_id);

/**
 * @brief Checks if the given timer is expired.
 *
 * @param timer_id the  timer identifier.
 *
 * @return True when the timer expired, false otherwise.
 */
bool ncs_pal_timer_is_expired(uint8_t timer_id);

/**
 * @brief Set delay in milliseconds.
 *
 * @param delay_ms the delay in milliseconds.
 */
void ncs_pal_delay(uint16_t delay_ms);

/**
 * @brief Get the remaining time of the timer.
 *
 * @param timer_id the timer identifier.
 *
 * @return Remaining time in units of system ticks, or 0 if the timer is not running.
 */
uint32_t ncs_pal_timer_get_remaining(uint8_t timer_id);

#ifdef __cplusplus
}
#endif

#endif /* NCS_PAL_TIMER_H */
