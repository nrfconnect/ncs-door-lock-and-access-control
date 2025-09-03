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
#include <stdio.h>
#include <zephyr/logging/log.h>

#define _ALIRO_LOG(level, ...)                                                                                         \
	do {                                                                                                           \
		if (level <= CONFIG_NCS_ALIRO_LOG_LEVEL_VALUE) {                                                       \
			_AliroPlatformLog(level, __VA_ARGS__);                                                         \
		}                                                                                                      \
	} while (0)

#define _ALIRO_LOG_HEXDUMP(level, data, size, str)                                                                     \
	do {                                                                                                           \
		if (level <= CONFIG_NCS_ALIRO_LOG_LEVEL_VALUE) {                                                       \
			_AliroPlatformLogHexdump(level, data, size, str);                                              \
		}                                                                                                      \
	} while (0)

#define ALIRO_LOG_ERR(...) _ALIRO_LOG(LOG_LEVEL_ERR, __VA_ARGS__)
#define ALIRO_LOG_WRN(...) _ALIRO_LOG(LOG_LEVEL_WRN, __VA_ARGS__)
#define ALIRO_LOG_INF(...) _ALIRO_LOG(LOG_LEVEL_INF, __VA_ARGS__)
#define ALIRO_LOG_DBG(...) _ALIRO_LOG(LOG_LEVEL_DBG, __VA_ARGS__)

#define ALIRO_LOG_HEXDUMP_ERR(data, size, str) _ALIRO_LOG_HEXDUMP(LOG_LEVEL_ERR, data, size, str)
#define ALIRO_LOG_HEXDUMP_WRN(data, size, str) _ALIRO_LOG_HEXDUMP(LOG_LEVEL_WRN, data, size, str)
#define ALIRO_LOG_HEXDUMP_INF(data, size, str) _ALIRO_LOG_HEXDUMP(LOG_LEVEL_INF, data, size, str)
#define ALIRO_LOG_HEXDUMP_DBG(data, size, str) _ALIRO_LOG_HEXDUMP(LOG_LEVEL_DBG, data, size, str)

/*
 * Helpers: Debug builds print hexdump, other levels print info logs.
 */
#ifdef CONFIG_NCS_ALIRO_LOG_LEVEL_DBG
#define ALIRO_LOG_DBG_HEXDUMP_OR_INFO(data, size, fmt, ...)                                                            \
	do {                                                                                                           \
		char _aliro_log_desc[128];                                                                             \
		(void)snprintf(_aliro_log_desc, sizeof(_aliro_log_desc), fmt, ##__VA_ARGS__);                          \
		ALIRO_LOG_HEXDUMP_DBG(data, size, _aliro_log_desc);                                                    \
	} while (0)
#else
#define ALIRO_LOG_DBG_HEXDUMP_OR_INFO(data, size, fmt, ...) ALIRO_LOG_INF(fmt, ##__VA_ARGS__)
#endif

/**
 * @brief Writes a log message with the specified log level.
 * @note The function is intended to be implemented by the platform.
 *
 * @param[in]  logLevel   the log level.
 * @param[in]  logFormat  a pointer to the format string.
 * @param[in]  ...        arguments for the format specification.
 */
void _AliroPlatformLog(uint8_t platformLogLevel, const char *logFormat, ...);

/**
 * @brief Logs binary data in hexadecimal format with the specified log level.
 * @note The function is intended to be implemented by the platform.
 *
 * @param[in] logLevel the log level.
 * @param[in] data     a pointer to the data to be logged.
 * @param[in] size     size of the data in bytes.
 * @param[in] str      description string to prefix the hexdump.
 */
void _AliroPlatformLogHexdump(uint8_t platformLogLevel, const void *data, size_t size, const char *str);

#endif // ALIRO_PLATFORM_LOG_H_
