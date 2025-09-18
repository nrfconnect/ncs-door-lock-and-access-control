/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "ncs_pal_gpio.h"
#include "ncs_pal_semaphore.h"

#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(pal_gpio, CONFIG_NFC_LOG_LEVEL);

static const struct gpio_dt_spec irq_gpio = GPIO_DT_SPEC_GET(DT_INST(0, x_nucleo_nfc), irq_gpios);
// The reset GPIO is optional, so we use GPIO_DT_SPEC_GET_OR to avoid errors if it is not defined.
static const struct gpio_dt_spec reset_gpio = GPIO_DT_SPEC_GET_OR(DT_INST(0, x_nucleo_nfc), reset_gpios, { 0 });

// TODO: This should be moved to the nRF54L style shield.
#if defined(CONFIG_BOARD_NRF54L15DK) || defined(CONFIG_BOARD_NRF54LM20DK)
static const struct gpio_dt_spec nfc_pwr_switch = GPIO_DT_SPEC_GET(DT_NODELABEL(nfc_power_switch), gpios);
#endif // CONFIG_BOARD_NRF54L15DK || CONFIG_BOARD_NRF54LM20DK

static void irq_pin_cb(const struct device *gpiob, struct gpio_callback *cb, uint32_t pins)
{
	ARG_UNUSED(gpiob);
	ARG_UNUSED(cb);
	ARG_UNUSED(pins);

	ncs_pal_give_semaphore();
}

// TODO: The function should be moved to the nRF54L style shield.
int ncs_pal_pwr_pin_set()
{
	int err = 0;
#if defined(CONFIG_BOARD_NRF54L15DK) || defined(CONFIG_BOARD_NRF54LM20DK)
	if (!gpio_is_ready_dt(&nfc_pwr_switch)) {
		LOG_ERR("The NFC Power switch pin GPIO port is not ready");
		return -ENODEV;
	}

	err = gpio_pin_configure_dt(&nfc_pwr_switch, GPIO_OUTPUT_HIGH);
	if (err) {
		LOG_ERR("Configuring GPIO pin failed: %d", err);
	}
#endif // CONFIG_BOARD_NRF54L15DK || CONFIG_BOARD_NRF54LM20DK
	return err;
}

int ncs_pal_gpio_init(void)
{
	static struct gpio_callback gpio_cb;
	int err = 0;

	LOG_DBG("GPIO init");

	if (reset_gpio.port) {
		if (!gpio_is_ready_dt(&reset_gpio)) {
			LOG_ERR("Reset GPIO device not ready");
			return -ENODEV;
		}

		/* Configure reset pin */
		err = gpio_pin_configure_dt(&reset_gpio, GPIO_OUTPUT_LOW);
		if (err) {
			return err;
		}
	}

	if (!gpio_is_ready_dt(&irq_gpio)) {
		LOG_ERR("IRQ GPIO device not ready");
		return -ENODEV;
	}

	/* Configure IRQ pin */
	err = gpio_pin_configure_dt(&irq_gpio, GPIO_INPUT);
	if (err) {
		return err;
	}

	gpio_init_callback(&gpio_cb, irq_pin_cb, BIT(irq_gpio.pin));

	err = gpio_add_callback(irq_gpio.port, &gpio_cb);
	if (err) {
		return err;
	};

	return gpio_pin_interrupt_configure_dt(&irq_gpio, GPIO_INT_EDGE_TO_ACTIVE);
}

bool ncs_pal_gpio_is_set(int port, int pin)
{
	ARG_UNUSED(pin);
	ARG_UNUSED(port);

	// TODO: Error handling
	int value = gpio_pin_get_dt(&irq_gpio);
	return value == 1;
}
