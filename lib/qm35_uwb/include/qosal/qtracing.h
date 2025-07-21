/*
 * Copyright (c) 2022-2023 Qorvo, Inc
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
