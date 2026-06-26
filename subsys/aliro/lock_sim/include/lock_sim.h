/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
#pragma once

#include "aliro/types.h"

#include <zephyr/kernel.h>

namespace Aliro {

class LockSim {
public:
	using LockStateChangeCallback = void (*)(ReaderStateByte state);

	LockSim() = default;
	~LockSim() = default;

	LockSim(const LockSim &) = delete;
	LockSim &operator=(const LockSim &) = delete;
	LockSim(LockSim &&) = delete;
	LockSim &operator=(LockSim &&) = delete;

	/**
	 * @brief Initializes the lock simulator.
	 *
	 * @param callback The callback to be called when the lock state changes.
	 */
	void Init(LockStateChangeCallback callback);

	/**
	 * @brief Locks the lock.
	 *
	 * @return True if the lock was locked successfully, false if the lock is already locked.
	 */
	bool Lock();

	/**
	 * @brief Unlocks the lock.
	 *
	 * @return True if the lock was unlocked successfully, false if the lock is already unlocked.
	 */
	bool Unlock();

private:
	static constexpr uint32_t kActuatorMovementTimeMs{ CONFIG_DOOR_LOCK_ALIRO_LOCK_SIM_MOVEMENT_TIME_MS };
#ifdef CONFIG_DOOR_LOCK_ALIRO_LOCK_SIM_AUTO_RELOCK
	static constexpr uint32_t kAutoRelockTimeMs{ CONFIG_DOOR_LOCK_ALIRO_LOCK_SIM_AUTO_RELOCK_TIME_MS };
#endif // CONFIG_DOOR_LOCK_ALIRO_LOCK_SIM_AUTO_RELOCK

	static void ActuatorTimerEventHandler(k_timer *timer);
	static void NotifyWorkHandler(k_work *work);
#ifdef CONFIG_DOOR_LOCK_ALIRO_LOCK_SIM_AUTO_RELOCK
	static void AutoRelockTimerEventHandler(k_timer *timer);
#endif // CONFIG_DOOR_LOCK_ALIRO_LOCK_SIM_AUTO_RELOCK

	void StartOperation(ReaderStateByte state);
	void ActuatorTimerEventHandler();
	void NotifyWorkHandler();
#ifdef CONFIG_DOOR_LOCK_ALIRO_LOCK_SIM_AUTO_RELOCK
	void StartAutoRelock();
	void AutoRelockTimerEventHandler();
#endif // CONFIG_DOOR_LOCK_ALIRO_LOCK_SIM_AUTO_RELOCK

	LockStateChangeCallback mLockStateChangeCallback{ nullptr };
	k_timer mActuatorTimer;
	k_work mNotifyWork;
	ReaderStateByte mState{ ReaderStateByte::Secured };
#ifdef CONFIG_DOOR_LOCK_ALIRO_LOCK_SIM_AUTO_RELOCK
	k_timer mAutoRelockTimer;
#endif // CONFIG_DOOR_LOCK_ALIRO_LOCK_SIM_AUTO_RELOCK
};

} // namespace Aliro
