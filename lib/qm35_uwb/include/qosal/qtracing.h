/*
 * SPDX-FileCopyrightText: Copyright (c) 2022 Qorvo, Inc.
 * SPDX-License-Identifier: LicenseRef-Qorvo-SDK-1
 */

#pragma once

#include "qerr.h"
#include "qosal_impl.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * typedef qtracing_cb_t() - Define a tracing callback.
 * @fmt: string format of the trace.
 * @...: variable list of arguments.
 *
 * Return: nothing.
 */
typedef void (*qtracing_cb_t)(const char *fmt, ...);

#ifndef QOSAL_PRINT_TRACE
extern qtracing_cb_t tracing_cb;
#define QOSAL_PRINT_TRACE(...)                   \
	do {                                     \
		if (tracing_cb)                  \
			tracing_cb(__VA_ARGS__); \
	} while (0)
#endif

/**
 * qtracing_init() - Initialize tracing.
 *
 * Return: QERR_SUCCESS or error.
 */
enum qerr qtracing_init(void);

#ifdef __cplusplus
}
#endif
