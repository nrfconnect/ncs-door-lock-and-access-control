/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include "board/board.h"
#include "board/led_widget.h"
#include "bolt_lock_manager.h"

struct k_timer;
struct Identify;

#ifdef CONFIG_NCS_SAMPLE_MATTER_TEST_EVENT_TRIGGERS
#include "event_triggers/event_triggers.h"
#endif

class AppTask {
public:
	static AppTask &Instance()
	{
		static AppTask sAppTask;
		return sAppTask;
	};

	CHIP_ERROR StartApp();

	void UpdateClusterState(const BoltLockManager::StateData &stateData);

private:
	CHIP_ERROR Init();

	static void LockActionEventHandler();
	static void ButtonEventHandler(Nrf::ButtonState state, Nrf::ButtonMask hasChanged);
	static void LockStateChanged(const BoltLockManager::StateData &stateData);
	static void UpdateClusterStateHandler(const BoltLockManager::StateData &stateData);

#ifdef CONFIG_DOOR_LOCK_NUS_SERVICE
	static void NUSLockCallback(void *context);
	static void NUSUnlockCallback(void *context);
#endif // CONFIG_DOOR_LOCK_NUS_SERVICE

#ifdef CONFIG_NCS_SAMPLE_MATTER_TEST_EVENT_TRIGGERS
	constexpr static Nrf::Matter::TestEventTrigger::EventTriggerId kDoorLockJammedEventTriggerId =
		0xFFFF'FFF3'2774'0000;
	static CHIP_ERROR DoorLockJammedEventCallback(Nrf::Matter::TestEventTrigger::TriggerValue);
#endif
};
