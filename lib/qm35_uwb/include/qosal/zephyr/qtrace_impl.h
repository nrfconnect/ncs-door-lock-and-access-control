/*
 * SPDX-FileCopyrightText: Copyright (c) 2024 Qorvo, Inc.
 * SPDX-License-Identifier: LicenseRef-Qorvo-SDK-1
 */

#ifndef QTRACE_IMPL_H
#define QTRACE_IMPL_H
/**
 * qtrace_gettime_us() - Return current time in micro-seconds.
 * @tstamp_us: Address where to put current time.
 *
 * This function MUST return monotonic time in micro-second into the provided
 * @tstamp pointer. This time MUST support sleep state so should be based on
 * RTC.
 *
 * Application must implement this function or @QTRACE_USE_POSIX_GETTIME must
 * be defined to 1 to use POSIX clock_gettime() by default to calculate current
 * time.
 *
 * Returns: 0 on success or a negative error.
 */
#include "qtime.h"
static inline int qtrace_gettime_us(uint64_t *tstamp_us)
{
	/*
	 * Getting time is consuming a lot of cpu so rely on ticks:
	 * qtime_get_uptime_us : ~5.25µs
	 * qtime_get_uptime_ticks : ~2.2µs
	 */
	*tstamp_us = qtime_get_uptime_ticks();
	return 0;
}

#endif // QTRACE_IMPL_H
