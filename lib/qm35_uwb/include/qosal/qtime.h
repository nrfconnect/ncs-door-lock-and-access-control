/*
 * SPDX-FileCopyrightText: Copyright (c) 2024 Qorvo, Inc.
 * SPDX-License-Identifier: LicenseRef-Qorvo-SDK-1
 */

#pragma once

#ifdef __KERNEL__
#include <linux/types.h>
#else
#include <stdint.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Option to define QTIME_* externally
 */
#include <qtime_impl.h>

/**
 *  QOSAL_WAIT_FOREVER - Timeout value to wait forever.
 */
#define QOSAL_WAIT_FOREVER 0xFFFFFFFFU

#ifdef QTIME_GET_STRING_TICK_PER_S
#define qtime_get_string_ticks_per_s() QTIME_GET_STRING_TICKS_PER_S()
#else
#define qtime_get_string_ticks_per_s() qtime_get_string_ticks_per_s_default()
/**
 * qtime_get_string_ticks_per_s_default() - Get tick per second (Hz) in string.
 *
 * Return: ticks per s in string.
 */
const char *qtime_get_string_ticks_per_s_default(void);
#endif

#ifdef QTIME_GET_UPTIME_TICKS
#define qtime_get_uptime_ticks() QTIME_GET_UPTIME_TICKS()
#else
#define qtime_get_uptime_ticks() qtime_get_uptime_ticks_default()
/**
 * qtime_get_uptime_ticks_default() - Get uptime in ticks.
 *
 * Return: uptime in ticks.
 */
int64_t qtime_get_uptime_ticks_default(void);
#endif

/**
 * qtime_get_uptime_us() - Get uptime in us.
 *
 * Return: uptime in us.
 */
int64_t qtime_get_uptime_us(void);

/**
 * qtime_msleep() - Sleep milliseconds.
 * @ms: Number of ms to sleep.
 */
void qtime_msleep(int ms);

/**
 * qtime_usleep() - Sleep microseconds.
 * @us: Number of us to sleep.
 */
void qtime_usleep(int us);

/**
 * qtime_get_sys_freq_hz() - Get system frequency in Hz.
 *
 * Return: system frequency in Hz.
 */
uint32_t qtime_get_sys_freq_hz(void);

#ifdef __cplusplus
}
#endif
