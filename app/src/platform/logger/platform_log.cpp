/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "logger/platform_log.h"

#include <zephyr/logging/log.h>

#include <stdarg.h>

LOG_MODULE_REGISTER(platform, CONFIG_NCS_DOOR_LOCK_APP_LOG_LEVEL);

void _AliroPlatformLogHexdump(uint8_t platformLogLevel, const void *data, size_t size, const char *str)
{
	switch (platformLogLevel) {
	case LOG_LEVEL_ERR:
		LOG_HEXDUMP_ERR(data, size, str);
		break;
	case LOG_LEVEL_WRN:
		LOG_HEXDUMP_WRN(data, size, str);
		break;
	case LOG_LEVEL_INF:
		LOG_HEXDUMP_INF(data, size, str);
		break;
	case LOG_LEVEL_DBG:
		LOG_HEXDUMP_DBG(data, size, str);
		break;
	case LOG_LEVEL_NONE:
	default:
		break;
	}
}

void _AliroPlatformLog(uint8_t platformLogLevel, const char *logFormat, ...)
{
#if defined(CONFIG_LOG) && !defined(CONFIG_LOG_MODE_MINIMAL)

	if (platformLogLevel > CONFIG_NCS_ALIRO_LOG_LEVEL_VALUE) {
		return;
	}

	va_list paramList;
	va_start(paramList, logFormat);
	log_generic(platformLogLevel, logFormat, paramList);
	va_end(paramList);

#else // defined(CONFIG_LOG) && !defined(CONFIG_LOG_MODE_MINIMAL)

	ARG_UNUSED(platformLogLevel);
	ARG_UNUSED(logFormat);

#endif // defined(CONFIG_LOG) && !defined(CONFIG_LOG_MODE_MINIMAL)
}
