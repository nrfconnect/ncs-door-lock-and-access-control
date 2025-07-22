/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "logger/platform_log.h"

#include <zephyr/logging/log.h>

#include <stdarg.h>

LOG_MODULE_REGISTER(platform, CONFIG_NCS_DOOR_LOCK_APP_LOG_LEVEL);

void AliroPlatformLogHexdump(AliroLogLevel logLevel, const void *data, size_t size, const char *str)
{
	switch (logLevel) {
	case ALIRO_LOG_LEVEL_ERROR:
		LOG_HEXDUMP_ERR(data, size, str);
		break;
	case ALIRO_LOG_LEVEL_WARN:
		LOG_HEXDUMP_WRN(data, size, str);
		break;
	case ALIRO_LOG_LEVEL_INFO:
		LOG_HEXDUMP_INF(data, size, str);
		break;
	case ALIRO_LOG_LEVEL_DEBUG:
		LOG_HEXDUMP_DBG(data, size, str);
		break;
	case ALIRO_LOG_LEVEL_NONE:
	default:
		break;
	}
}

void AliroPlatformLog(AliroLogLevel logLevel, const char *logFormat, ...)
{
#if defined(CONFIG_LOG) && !defined(CONFIG_LOG_MODE_MINIMAL)

	uint8_t platformLogLevel{ LOG_LEVEL_NONE };

	switch (logLevel) {
	case ALIRO_LOG_LEVEL_ERROR:
		platformLogLevel = LOG_LEVEL_ERR;
		break;
	case ALIRO_LOG_LEVEL_WARN:
		platformLogLevel = LOG_LEVEL_WRN;
		break;
	case ALIRO_LOG_LEVEL_INFO:
		platformLogLevel = LOG_LEVEL_INF;
		break;
	case ALIRO_LOG_LEVEL_DEBUG:
		platformLogLevel = LOG_LEVEL_DBG;
		break;
	case ALIRO_LOG_LEVEL_NONE:
	default:
		platformLogLevel = LOG_LEVEL_NONE;
	}

	va_list paramList;
	va_start(paramList, logFormat);
	log_generic(platformLogLevel, logFormat, paramList);
	va_end(paramList);

#else // defined(CONFIG_LOG) && !defined(CONFIG_LOG_MODE_MINIMAL)

	ARG_UNUSED(logLevel);
	ARG_UNUSED(logFormat);

#endif // defined(CONFIG_LOG) && !defined(CONFIG_LOG_MODE_MINIMAL)
}
