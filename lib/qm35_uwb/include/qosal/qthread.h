/**
 * @file      qthread.h
 *
 * @brief     Header file for Qorvo thread management
 *
 * @author    Qorvo Paris Applications
 *
 * @copyright SPDX-FileCopyrightText: Copyright (c) 2024 Qorvo, Inc.
 *            SPDX-License-Identifier: LicenseRef-Qorvo-SDK-1
 *
 */

#pragma once

#include <qerr.h>
#include <qosal_impl.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * QTHREAD_STACK_DEFINE() - Statically allocate a thread stack.
 * @name: Name of the stack
 * @stack_size: Stack size
 */
#define QTHREAD_STACK_DEFINE(name, stack_size) QOSAL_IMPL_THREAD_STACK_DEFINE(name, stack_size)

/**
 * QALIGN() - Perform a byte alignment.
 * @size: Initial size.
 * @byte: Number of byte boundary for the alignment.
 */
#define QALIGN(size, byte) ((size + byte - 1) & ~(byte - 1))

/**
 * enum qthread_priority - QOSAL Thread priority.
 * @QTHREAD_PRIORITY_CRITICAL: Critical priority (maximum).
 * @QTHREAD_PRIORITY_HIGH: High priority.
 * @QTHREAD_PRIORITY_ABOVE_NORMAL: Above normal priority.
 * @QTHREAD_PRIORITY_NORMAL: Normal priority.
 * @QTHREAD_PRIORITY_BELOW_NORMAL: Below normal priority.
 * @QTHREAD_PRIORITY_LOW: Low priority.
 * @QTHREAD_PRIORITY_IDLE: Idle priority (minimum).
 * @QTHREAD_PRIORITY_MAX: Internal use.
 */
enum qthread_priority {
	QTHREAD_PRIORITY_CRITICAL = 0,
	QTHREAD_PRIORITY_HIGH = 1,
	QTHREAD_PRIORITY_ABOVE_NORMAL = 2,
	QTHREAD_PRIORITY_NORMAL = 3,
	QTHREAD_PRIORITY_BELOW_NORMAL = 4,
	QTHREAD_PRIORITY_LOW = 5,
	QTHREAD_PRIORITY_IDLE = 6,
	QTHREAD_PRIORITY_MAX = 7,
};

/*
 * struct qthread: QOSAL Thread (opaque)
 */
struct qthread;

/**
 * typedef qthread_func - Pointer to a thread entry point.
 * @arg: private data of the thread.
 *
 * Return: nothing.
 */
typedef void (*qthread_func)(void *arg);

/**
 * qthread_create() - Create a new thread.
 * @thread: Entry point of the thread.
 * @arg: Private data of the thread.
 * @name: Name of the thread.
 * @stack: Pointer to the stack of the thread.
 * @stack_size: Size of the stack of the thread.
 * @prio: Priority of the thread.
 *
 * NOTE: If stack is NULL, it will be automatically allocated.
 *
 * Return: Pointer to the created thread.
 */
struct qthread *qthread_create(qthread_func thread, void *arg, const char *name, void *stack,
			       uint32_t stack_size, enum qthread_priority prio);

/**
 * qthread_join() - Wait for the thread to exit.
 * @thread: Pointer to the thread.
 *
 * Return: QERR_SUCCESS or error.
 */
enum qerr qthread_join(struct qthread *thread);

/**
 * qthread_delete() - Delete a running thread.
 * @thread: Pointer to the thread.
 *
 * Return: QERR_SUCCESS or error.
 */
enum qerr qthread_delete(struct qthread *thread);

#ifdef __cplusplus
}
#endif
