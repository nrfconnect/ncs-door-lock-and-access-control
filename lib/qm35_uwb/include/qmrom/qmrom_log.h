/*
 * SPDX-FileCopyrightText: Copyright (c) 2021 Qorvo, Inc.
 * SPDX-License-Identifier: GPL-2.0 OR Apache-2.0
 */

#pragma once

enum log_level_e {
	LOG_QUIET = 0,
	LOG_ERR,
	LOG_WARN,
	LOG_INFO,
	LOG_DBG,
	LOG_LVL_MAX = LOG_DBG,
};

extern enum log_level_e __log_level__;

void set_log_level(enum log_level_e lvl);

static inline int is_debug_mode(void)
{
	return __log_level__ >= LOG_DBG;
}

static inline int is_log_level_allowed(enum log_level_e lvl)
{
	return (__log_level__ >= lvl);
}

void hexdump(enum log_level_e lvl, const void *address, unsigned short length);
void hexrawdump(enum log_level_e lvl, const void *address,
		unsigned short length);

#if defined(__KERNEL__)

#include <linux/device.h>

extern struct device *__qmrom_log_dev__;

void qmrom_set_log_device(struct device *dev, enum log_level_e lvl);

#define LOG_ERR(...)                                                     \
	do {                                                             \
		if (__qmrom_log_dev__)                                   \
			if (__log_level__ >= LOG_ERR)                    \
				dev_err(__qmrom_log_dev__, __VA_ARGS__); \
	} while (0)
#define LOG_WARN(...)                                                     \
	do {                                                              \
		if (__qmrom_log_dev__)                                    \
			if (__log_level__ >= LOG_WARN)                    \
				dev_warn(__qmrom_log_dev__, __VA_ARGS__); \
	} while (0)
#define LOG_INFO(...)                                                     \
	do {                                                              \
		if (__qmrom_log_dev__)                                    \
			if (__log_level__ >= LOG_INFO)                    \
				dev_info(__qmrom_log_dev__, __VA_ARGS__); \
	} while (0)
#define LOG_DBG(...)                                                     \
	do {                                                             \
		if (__qmrom_log_dev__)                                   \
			if (__log_level__ >= LOG_DBG)                    \
				dev_dbg(__qmrom_log_dev__, __VA_ARGS__); \
	} while (0)

#elif defined(__ANDROID__) && defined(HAVE_ANDROID_LOGGING)

#define LOG_TAG "qorvo.qmrom"
#include <log/log.h>
#define LOG_ERR(...) ALOGE_IF(__log_level__ > LOG_QUIET, __VA_ARGS__)
#define LOG_WARN(...) ALOGW_IF(__log_level__ > LOG_ERR, __VA_ARGS__)
#define LOG_INFO(...) ALOGI_IF(__log_level__ > LOG_WARN, __VA_ARGS__)
#define LOG_DBG(...) ALOGD_IF(__log_level__ > LOG_INFO, __VA_ARGS__)

#else

#include <stdio.h>
#define LOG_ERR(...)                                  \
	do {                                          \
		if (__log_level__ > LOG_QUIET)        \
			fprintf(stderr, __VA_ARGS__); \
	} while (0)
#define LOG_WARN(...)                                 \
	do {                                          \
		if (__log_level__ > LOG_ERR)          \
			fprintf(stderr, __VA_ARGS__); \
	} while (0)
#define LOG_INFO(...)                                 \
	do {                                          \
		if (__log_level__ > LOG_WARN)         \
			fprintf(stdout, __VA_ARGS__); \
	} while (0)
#define LOG_DBG(...)                                  \
	do {                                          \
		if (__log_level__ > LOG_INFO)         \
			fprintf(stdout, __VA_ARGS__); \
	} while (0)

#endif
