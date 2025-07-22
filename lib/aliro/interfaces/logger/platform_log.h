/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

#ifndef ALIRO_PLATFORM_LOG_H_
#define ALIRO_PLATFORM_LOG_H_

#include <stdarg.h>
#include <stddef.h>
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
 * @param[in]  logLevel   the log level.
 * @param[in]  logFormat  a pointer to the format string.
 * @param[in]  ...        arguments for the format specification.
 */
void AliroPlatformLog(AliroLogLevel logLevel, const char *logFormat, ...);

/**
 * @brief Logs binary data in hexadecimal format with the specified log level.
 * @note The function is intended to be implemented by the platform.
 *
 * @param[in] logLevel the log level.
 * @param[in] data     a pointer to the data to be logged.
 * @param[in] size     size of the data in bytes.
 * @param[in] str      description string to prefix the hexdump.
 */
void AliroPlatformLogHexdump(AliroLogLevel logLevel, const void *data, size_t size, const char *str);

#endif // ALIRO_PLATFORM_LOG_H_
