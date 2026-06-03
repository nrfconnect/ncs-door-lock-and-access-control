/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "aliro/interface.h"

#include <aliro_workqueue/aliro_workqueue.h>

#include <zephyr/init.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <array>
#include <tuple>

LOG_MODULE_REGISTER(interface_os_timer, CONFIG_DOOR_LOCK_APP_LOG_LEVEL);

namespace Aliro::Interface::Os::Timer {

namespace {

constexpr size_t kNfcStackTimers{ 1 };

#ifdef CONFIG_DOOR_LOCK_BLE_UWB
constexpr size_t kBleStackTimers{ CONFIG_DOOR_LOCK_BLE_UWB_MAX_SESSIONS };
#else
constexpr size_t kBleStackTimers{ 0 };
#endif

constexpr size_t kStackTimerCount{ kNfcStackTimers + kBleStackTimers };

struct TimerSlot {
	k_timer mTimer{};
	k_work mWork{};
	Callback mCallback{ nullptr };
	void *mContext{ nullptr };
};

std::array<TimerSlot, kStackTimerCount> sTimerSlots{};

bool IsHandleValid(Handle handle)
{
	return handle >= 0 && handle < static_cast<int>(sTimerSlots.size());
}

TimerSlot *GetSlot(Handle handle)
{
	if (IsHandleValid(handle)) {
		auto &slot = sTimerSlots[handle];
		if (!slot.mCallback) {
			LOG_ERR("Timer slot is not initialized (handle: %d)", handle);
			return nullptr;
		}
		return &slot;
	}

	LOG_ERR("Invalid timer slot (handle: %d)", handle);
	return nullptr;
}

void WorkHandler(k_work *work)
{
	auto &slot = *CONTAINER_OF(work, TimerSlot, mWork);

	if (slot.mCallback) {
		slot.mCallback(slot.mContext);
	}
}

void ExpiryHandler(k_timer *timer)
{
	auto *slot = static_cast<TimerSlot *>(k_timer_user_data_get(timer));
	if (!slot) {
		return;
	}

	const auto err = AliroWorkqueueSubmit(&slot->mWork);
	if (err < 0) {
		LOG_ERR("Failed to submit timer expiry work, err: %d", err);
	}
}

int InitTimerSlots(void)
{
	for (auto &slot : sTimerSlots) {
		k_timer_init(&slot.mTimer, ExpiryHandler, nullptr);
		k_timer_user_data_set(&slot.mTimer, &slot);
		k_work_init(&slot.mWork, WorkHandler);
	}

	return 0;
}

} // namespace

Handle Acquire(Callback callback, void *context)
{
	if (!callback) {
		LOG_ERR("Timer callback must not be null");
		return kInvalidHandle;
	}

	for (size_t index = 0; index < kStackTimerCount; index++) {
		auto &slot = sTimerSlots[index];
		if (!slot.mCallback) {
			slot.mCallback = callback;
			slot.mContext = context;
			return static_cast<Handle>(index);
		}
	}

	LOG_ERR("No free stack timer slot (count: %zu)", kStackTimerCount);
	return kInvalidHandle;
}

void Release(Handle handle)
{
	const auto slot{ GetSlot(handle) };
	if (!slot) {
		return;
	}

	/* Mark slot as unused first so pending work will not execute callback. */
	slot->mCallback = nullptr;
	slot->mContext = nullptr;

	k_timer_stop(&slot->mTimer);
	k_work_sync sync{};
	k_work_cancel_sync(&slot->mWork, &sync);
}

void Start(Handle handle, uint32_t timeoutMs)
{
	const auto slot{ GetSlot(handle) };
	if (!slot) {
		return;
	}

	k_timer_start(&slot->mTimer, K_MSEC(timeoutMs), K_NO_WAIT);
}

void Stop(Handle handle)
{
	const auto slot{ GetSlot(handle) };
	if (!slot) {
		return;
	}

	k_timer_stop(&slot->mTimer);
}

bool IsRunning(Handle handle)
{
	const auto slot{ GetSlot(handle) };
	if (!slot) {
		return false;
	}

	return k_timer_remaining_ticks(&slot->mTimer) != 0;
}

SYS_INIT(InitTimerSlots, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);

} // namespace Aliro::Interface::Os::Timer
