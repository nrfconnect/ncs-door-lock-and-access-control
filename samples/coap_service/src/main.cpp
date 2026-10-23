/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <atomic>
#include <coap_service/coap_service.h>
#include <dk_buttons_and_leds.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

using State = DoorLock::CoAPService::State;

static std::atomic<State> sState{ State::Locked };

static void ToggleState()
{
	sState = sState == State::Locked ? State::Unlocked : State::Locked;
	printk("Door %s\n", sState == State::Unlocked ? "unlocked" : "locked");
	dk_set_led(DK_LED1, (sState == State::Locked));
}

static State GetState()
{
	return sState;
}

static void SetState(State newState)
{
	sState = newState;
	printk("Door %s\n", sState == State::Unlocked ? "unlocked" : "locked");
	dk_set_led(DK_LED1, (sState == State::Locked));
}

static void ButtonChanged(uint32_t buttonState, uint32_t changedButtons)
{
	if (buttonState & changedButtons & DK_BTN1_MSK) {
		ToggleState();
	}
}

int main()
{
	DoorLock::CoAPService::Init(GetState, SetState);

	int err = dk_buttons_init(ButtonChanged);
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

	SetState(State::Locked);

	printk("Button 0 toggles the lock.\n"
	       "Connect with: wifi connect -s \"SSID\" -k 1 -p \"PASSWORD\"\n"
	       "Use net ipv4 to find the address; CoAP: /%s, UDP %u.\n",
	       CONFIG_DOOR_LOCK_COAP_RESOURCE_PATH, CONFIG_DOOR_LOCK_COAP_SERVICE_PORT);
	return 0;
}
