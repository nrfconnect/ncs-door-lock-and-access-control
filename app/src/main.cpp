/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifdef CONFIG_CHIP
#include "matter/init.h"
#else // CONFIG_CHIP
#include "aliro/init.h"
#endif // CONFIG_CHIP

#include "aliro/utils.h"

#include <zephyr/logging/log.h>

#include <cstdlib>

#ifdef CONFIG_CHIP
LOG_MODULE_REGISTER(app, CONFIG_CHIP_APP_LOG_LEVEL);
#else // CONFIG_CHIP
LOG_MODULE_REGISTER(door_lock_app, CONFIG_NCS_DOOR_LOCK_APP_LOG_LEVEL);
#endif // CONFIG_CHIP

int main()
{
#ifdef CONFIG_CHIP

	int err = StartMatter();
	VerifyOrDie(err == EXIT_SUCCESS, "Failed to start Matter");

#else // CONFIG_CHIP

	int err = InitAliro();
	VerifyOrDie(err == EXIT_SUCCESS, "Failed to initialize Aliro");

	LOG_INF("Application started");

#endif // CONFIG_CHIP

	return EXIT_SUCCESS;
}
