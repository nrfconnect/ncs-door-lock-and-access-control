/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <aliro/interface.h>

#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(aliro_stack, CONFIG_NCS_ALIRO_LOG_LEVEL_VALUE);

namespace Aliro::Interface::Logging {

void Log(uint8_t platformLogLevel, const char *logFormat, ...)
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

void LogHexdump(uint8_t platformLogLevel, const void *data, size_t size, const char *str)
{
#if defined(CONFIG_LOG)

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

#else // defined(CONFIG_LOG)

	ARG_UNUSED(platformLogLevel);
	ARG_UNUSED(data);
	ARG_UNUSED(size);
	ARG_UNUSED(str);

#endif // defined(CONFIG_LOG)
}

} // namespace Aliro::Interface::Logging
