/*
 * SPDX-FileCopyrightText: Copyright (c) 2021 Qorvo, Inc.
 * SPDX-License-Identifier: GPL-2.0 OR Apache-2.0
 */

#pragma once

#if defined(__KERNEL__)

#include <linux/delay.h>
#include <linux/ktime.h>
#include <linux/mm.h>
#include <linux/slab.h>

#else /* __KERNEL__ */

#if defined(__ZEPHYR__)
#include <zephyr/kernel.h>
#else /* __ZEPHYR__ */
#include <stddef.h>
#endif /* __ZEPHYR__ */

#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

#endif /* __KERNEL__ */

/********************/
/* Helper Functions */
/********************/

struct firmware;
struct firmware *qmrom_load_firmware(const char *path);
#ifdef HAVE_HSSPI_UCI
int qmrom_update_firmware_uci_helper(char *name);
#endif

/******************/
/* Dynamic Memory */
/******************/

#if defined(__KERNEL__)

#define qmrom_alloc(size) kzalloc(size, GFP_KERNEL | GFP_DMA)
#define qmrom_free(ptr) kfree(ptr)
#define qmrom_data_dma_able(ptr) (!is_vmalloc_addr(ptr))

#else /* __KERNEL__ */

#define qmrom_alloc(size) calloc(1, size)
#define qmrom_free(ptr) free(ptr)
#define qmrom_data_dma_able(ptr) true

#endif /* __KERNEL__ */

/*********/
/* sleep */
/*********/

#if defined(__KERNEL__)

#define qmrom_usleep(us) usleep_range(us, (us)*11 / 10)
#define qmrom_msleep(ms) usleep_range((ms)*1000, (ms)*1100)

#elif defined(__ZEPHYR__) /* __KERNEL__ */

#define qmrom_usleep(us) k_usleep(us)
#define qmrom_msleep(ms) k_usleep((ms * 1000))

#else /* __KERNEL__ */

#define qmrom_usleep(us) usleep(us)
#define qmrom_msleep(ms) usleep((ms)*1000)

#endif /* __KERNEL__ */

/********/
/* time */
/********/

#if defined(__KERNEL__)

#define qmrom_time ktime_t
#define qmrom_time_get() ktime_get()
#define qmrom_time_sub(a, b) ktime_sub(a, b)
#define qmrom_time_div(a, b) ktime_divns(a, b)
#define qmrom_time_to_ns(t) ktime_to_ns(t)

#else /* __KERNEL__ */

#define qmrom_time uint64_t
#define qmrom_time_sub(a, b) ((a) - (b))
#define qmrom_time_div(a, b) ((a) / (b))

/* XCode 8 implements clock_gettime() and defines CLOCK_MONOTONIC_RAW,
 * even if it does not define _POSIX_TIMERS. */
#if defined(_POSIX_TIMERS) && _POSIX_TIMERS > 0 || defined(__APPLE__)

#include <time.h>

static inline qmrom_time qmrom_time_get()
{
	struct timespec time;
	clock_gettime(CLOCK_MONOTONIC_RAW, &time);
	return time.tv_sec * 1000000000 + time.tv_nsec;
}
#define qmrom_time_to_ns(t) (t)

#elif defined(_WIN32) /* _POSIX_TIMERS */

#include <sysinfoapi.h>

static inline qmrom_time qmrom_time_get()
{
	FILETIME time;
	GetSystemTimeAsFileTime(&time);
	return ((qmrom_time)time.dwHighDateTime << 32) + time.dwLowDateTime;
}
#define qmrom_time_to_ns(t) ((t)*100)

#elif defined(__ZEPHYR__) /* _POSIX_TIMERS */

static inline qmrom_time qmrom_time_get()
{
	return k_ticks_to_us_floor64(k_uptime_ticks());
}
#define qmrom_time_to_ns(t) ((t)*1000)

#else /* _POSIX_TIMERS */
#error "_POSIX_TIMERS not defined on this platform."
#endif /* _POSIX_TIMERS */

#endif /* __KERNEL__ */

/*******/
/* min */
/*******/

#if !defined(min)

#define min(a, b)                       \
	({                              \
		__typeof__(a) _a = (a); \
		__typeof__(b) _b = (b); \
		_a < _b ? _a : _b;      \
	})

#endif /* min */

/****************/
/* container_of */
/****************/

#if !defined(__KERNEL__)

#define container_of(ptr, type, member) \
	((type *)((char *)(ptr)-offsetof(type, member)))

#endif /* __KERNEL__ */
