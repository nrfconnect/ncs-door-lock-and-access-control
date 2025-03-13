/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef ALIRO_PLATFORM_LOG_H_
#define ALIRO_PLATFORM_LOG_H_

#include <stdarg.h>
#include <stdint.h>

/**
 * Log levels.
 */
typedef enum AliroLogLevel {
	ALIRO_LOG_LEVEL_NONE = 0,
	ALIRO_LOG_LEVEL_ERROR = 1,
	ALIRO_LOG_LEVEL_WARN = 2,
	ALIRO_LOG_LEVEL_INFO = 3,
	ALIRO_LOG_LEVEL_DEBUG = 4,
} AliroLogLevel;

/**
 * @brief Writes a log message with the specified log level.
 * @note The function is intended to be implemented by the platform.
 *
 * @param[in]  logLevel   The log level.
 * @param[in]  logFormat  A pointer to the format string.
 * @param[in]  ...        Arguments for the format specification.
 *
 */
void AliroPlatformLog(AliroLogLevel logLevel, const char *logFormat, ...);

#endif // ALIRO_PLATFORM_LOG_H_
