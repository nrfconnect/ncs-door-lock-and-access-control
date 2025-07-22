/*
 * Copyright (c) 2023 Qorvo, Inc
 *
 * All rights reserved.
 *
 * NOTICE: All information contained herein is, and remains the property
 * of Qorvo, Inc. and its suppliers, if any. The intellectual and technical
 * concepts herein are proprietary to Qorvo, Inc. and its suppliers, and
 * may be covered by patents, patent applications, and are protected by
 * trade secret and/or copyright law. Dissemination of this information
 * or reproduction of this material is strictly forbidden unless prior written
 * permission is obtained from Qorvo, Inc.
 *
 */

#pragma once

#include <qosal_impl.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define QOSAL_THREAD_MAX_NAME_LEN QOSAL_IMPL_THREAD_MAX_NAME_LEN

/**
 * struct qmemstats - Global memory statistics.
 * @static_size: Size of statically allocated memory.
 * @heap_used: Current amount of dynamically allocated memory.
 * @heap_peak: Peak amount of dynamically allocated memory.
 * @heap_size: Size of heap for dynamic memory allocation.
 *
 * A negative value indicates that the data could not be retrieved.
 */
struct qmemstats {
	int32_t static_size;
	int32_t heap_used;
	int32_t heap_peak;
	int32_t heap_size;
};

/**
 * struct qstackstats: Per stack memory statistics.
 * @thread_name: Name of the thread using the stack (if available).
 * @stack_used: Current stack usage.
 * @stack_peak: Peak stack usage.
 * @stack_size: Size of stack.
 *
 * A negative value indicates that the data could not be retrieved.
 */
struct qstackstats {
#if QOSAL_THREAD_MAX_NAME_LEN > 0
	char thread_name[QOSAL_THREAD_MAX_NAME_LEN];
#endif
	int32_t stack_used;
	int32_t stack_peak;
	int32_t stack_size;
};

/**
 * qmemstat_get() - Get global memory statistics.
 * @stats: Pointer to the struct to fill with memory statistics.
 */
void qmemstat_get(struct qmemstats *stats);

/**
 * qmemstat() - Display peak memory allocated with qmalloc().
 */
void qmemstat(void);

/**
 * qstackstat_count_get() - Get number of stacks.
 *
 * Return: Number of stacks handled by the OS.
 */
int qstackstat_count_get(void);

/**
 * qstackstat_get() - Get per stack memory statistics.
 * @stats: Pointer to the array of structs to fill with memory statistics.
 * @stack_count: number of allocated structs in the array.
 *
 * Return: Number of structs actually filled by the function.
 */
int qstackstat_get(struct qstackstats *stats, int stack_count);

/**
 * qstackstat() - Display peak stack usage per thread.
 */
void qstackstat(void);

/**
 * qprofstat() - Call others qprofiling functions.
 * Include qmemstat(), qstackstat().
 */
void qprofstat(void);

#ifdef __cplusplus
}
#endif
