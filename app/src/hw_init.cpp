/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <stdlib.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/init.h>
#include <zephyr/kernel.h>

#include "utils.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(hw_init, CONFIG_NCS_ALIRO_LOG_LEVEL);

static const struct gpio_dt_spec sNfcPowerSwitch = GPIO_DT_SPEC_GET(DT_NODELABEL(nfc_power_switch), gpios);

static int NfcResetPinConfigure()
{
	VerifyOrReturnStatus(gpio_is_ready_dt(&sNfcPowerSwitch), EXIT_FAILURE,
			     LOG_ERR("The NFC Power switch pin GPIO port is not ready"));

	// Configure PIN as output with high level = NFC tag is enabled by default.
	int erc = gpio_pin_configure_dt(&sNfcPowerSwitch, GPIO_OUTPUT_HIGH);
	VerifyOrReturnStatus(erc == 0, EXIT_FAILURE, LOG_ERR("Configuring GPIO pin failed: %d", erc));

	return EXIT_SUCCESS;
}

static int HardwareInit(void)
{
	return NfcResetPinConfigure();
}

SYS_INIT(HardwareInit, POST_KERNEL, CONFIG_APPLICATION_INIT_PRIORITY);
