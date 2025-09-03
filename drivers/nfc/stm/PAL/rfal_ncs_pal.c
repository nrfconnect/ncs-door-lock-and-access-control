/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "rfal_ncs_pal.h"

#include "ncs_pal_gpio.h"
#include "ncs_pal_spi.h"
#include "ncs_pal_timer.h"

#include <stdint.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(ncs_pal, CONFIG_NFC_LOG_LEVEL);

int rfal_ncs_pal_init(void)
{
	int err = ncs_pal_spi_init();
	if (err) {
		LOG_ERR("NFC PAL spi init failed %d", err);
		return err;
	}

	err = ncs_pal_pwr_pin_set();
	if (err) {
		LOG_ERR("NFC PAL gpio init failed %d", err);
		return err;
	}

	ncs_pal_timers_init();

	LOG_DBG("NCS PAL initialized");
	return 0;
}
