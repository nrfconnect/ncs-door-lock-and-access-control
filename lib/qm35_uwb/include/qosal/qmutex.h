/*
 * SPDX-FileCopyrightText: Copyright (c) 2024 Qorvo, Inc.
 * SPDX-License-Identifier: LicenseRef-Qorvo-SDK-1
 */

#pragma once

#include "qerr.h"
#include "qtime.h"

#ifndef __KERNEL__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * struct qmutex: QOSAL Mutex (opaque).
 */
struct qmutex;

/**
 * qmutex_init() - Initialize a mutex.
 *
 * Return: Pointer to the initialized mutex.
 */
struct qmutex *qmutex_init(void);

/**
 * qmutex_deinit() - De-initialize a mutex.
 * @mutex: Pointer to the mutex.
 */
void qmutex_deinit(struct qmutex *mutex);

/**
 * qmutex_lock() - Lock a mutex.
 * @mutex: Pointer to the mutex.
 * @timeout_ms: Delay until timeout in ms. Use `QOSAL_WAIT_FOREVER` to wait
 * indefinitely.
 *
 * Return: QERR_SUCCESS or error.
 */
enum qerr qmutex_lock(struct qmutex *mutex, uint32_t timeout_ms);

/**
 * qmutex_unlock() - Unlock a mutex.
 * @mutex: Pointer to the mutex.
 *
 * Return: QERR_SUCCESS or error.
 */
enum qerr qmutex_unlock(struct qmutex *mutex);

#ifdef __cplusplus
}
#endif

#else /* __KERNEL__ */

#include <linux/semaphore.h>
#include <linux/slab.h>
#include <linux/types.h>

/* Direct use of Linux kernel */
struct qmutex {
	struct semaphore s;
};

static inline struct qmutex *qmutex_init(void)
{
	struct qmutex *m = kmalloc(sizeof(*m), GFP_KERNEL);
	if (!m)
		return NULL;
	sema_init(&m->s, 1);
	return m;
}

static inline void qmutex_deinit(struct qmutex *mutex)
{
	kfree(mutex);
}

static inline enum qerr qmutex_lock(struct qmutex *mutex, uint32_t timeout_ms)
{
	int ret = down_timeout(&mutex->s, msecs_to_jiffies(timeout_ms));
	if (ret)
		return QERR_ETIME;
	return QERR_SUCCESS;
}

static inline enum qerr qmutex_unlock(struct qmutex *mutex)
{
	up(&mutex->s);
	return QERR_SUCCESS;
}

#endif /* __KERNEL__ */
