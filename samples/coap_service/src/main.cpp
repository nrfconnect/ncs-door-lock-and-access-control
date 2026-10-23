/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <coap_service/coap_service.h>
#include <dk_buttons_and_leds.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

static bool unlocked;
K_MUTEX_DEFINE(state_mutex);

static void toggle_state()
{
	k_mutex_lock(&state_mutex, K_FOREVER);
	unlocked = !unlocked;
	printk("Door %s\n", unlocked ? "unlocked" : "locked");
	dk_set_led(DK_LED1, !unlocked);
	k_mutex_unlock(&state_mutex);
}

static bool get_state()
{
	k_mutex_lock(&state_mutex, K_FOREVER);
	bool state = unlocked;
	k_mutex_unlock(&state_mutex);
	return state;
}

static void set_state(bool state)
{
	k_mutex_lock(&state_mutex, K_FOREVER);
	unlocked = state;
	printk("Door %s\n", unlocked ? "unlocked" : "locked");
	dk_set_led(DK_LED1, !unlocked);
	k_mutex_unlock(&state_mutex);
}

static void button_changed(uint32_t state, uint32_t changed)
{
	if (state & changed & DK_BTN1_MSK) {
		toggle_state();
	}
}

int main()
{
	DoorLock::CoAPService::Init(get_state, set_state);

	int err = dk_buttons_init(button_changed);
	if (err) {
		printk("Button initialization failed: %d\n", err);
		return err;
	}

	err = dk_leds_init();
	if (err) {
		printk("LED initialization failed: %d\n", err);
		return err;
	}

	err = DoorLock::CoAPService::Start();
	if (err) {
		printk("CoAP service start failed: %d\n", err);
		return err;
	}

	set_state(false);

	printk("Button 0 toggles the lock.\n"
	       "Connect with: wifi connect -s \"SSID\" -k 1 -p \"PASSWORD\"\n"
	       "Use net ipv4 to find the address; CoAP: /door-lock, UDP 5683.\n");
	return 0;
}
