/*
 * SPDX-FileCopyrightText: Copyright (c) 2024 Qorvo, Inc.
 * SPDX-License-Identifier: LicenseRef-Qorvo-SDK-1
 */

#pragma once

#include <qosal_impl.h>

#ifdef __cplusplus
extern "C" {
#endif

#define QLOG_LEVEL_NONE 0U
#define QLOG_LEVEL_ERR 1U
#define QLOG_LEVEL_WARN 2U
#define QLOG_LEVEL_INFO 3U
#define QLOG_LEVEL_DEBUG 4U

#ifndef QLOG_CURRENT_LEVEL

#ifndef CONFIG_QLOG_LEVEL
#define CONFIG_QLOG_LEVEL QLOG_LEVEL_WARN
#endif
/**
 *  QLOG_CURRENT_LEVEL - Log level to be defined by the user.
 *  Possible values are: QLOG_LEVEL_NONE, QLOG_LEVEL_ERR, QLOG_LEVEL_WARN,
 *  QLOG_LEVEL_INFO or QLOG_LEVEL_DEBUG.
 */
#define QLOG_CURRENT_LEVEL CONFIG_QLOG_LEVEL
#endif

#ifndef LOG_TAG
/**
 *  LOG_TAG - Log tag to be defined by the user.
 */
#define LOG_TAG ""
#endif

#define QLOG_LEVEL_CHECKED(level, ...)                          \
	do {                                                    \
		if (QLOG_LEVEL_##level <= QLOG_CURRENT_LEVEL) { \
			QOSAL_IMPL_LOG_##level(__VA_ARGS__);    \
		}                                               \
	} while (0)

/**
 *  QLOGD() - Print a debug log.
 */
#define QLOGD(...) QLOG_LEVEL_CHECKED(DEBUG, __VA_ARGS__)

/**
 *  QLOGE() - Print an error log.
 */
#define QLOGE(...) QLOG_LEVEL_CHECKED(ERR, __VA_ARGS__)

/**
 *  QLOGI() - Print an information log.
 */
#define QLOGI(...) QLOG_LEVEL_CHECKED(INFO, __VA_ARGS__)

/**
 *  QLOGW() - Print a warning log.
 */
#define QLOGW(...) QLOG_LEVEL_CHECKED(WARN, __VA_ARGS__)

#ifdef __cplusplus
}
#endif
