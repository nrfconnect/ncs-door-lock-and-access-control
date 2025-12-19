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
	using LockStateChangeCallback = void (*)(OperationSource source, ReaderStateByte state);

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
	 * @param source The source of the lock operation.
	 *
	 * @return True if the lock was locked successfully, false if the lock is already locked.
	 */
	bool Lock(OperationSource source);

	/**
	 * @brief Unlocks the lock.
	 *
	 * @param source The source of the unlock operation.
	 *
	 * @return True if the lock was unlocked successfully, false if the lock is already unlocked.
	 */
	bool Unlock(OperationSource source);

private:
	static constexpr uint32_t kActuatorMovementTimeMs{ CONFIG_DOOR_LOCK_LOCK_SIM_MOVEMENT_TIME_MS };
#ifdef CONFIG_DOOR_LOCK_LOCK_SIM_AUTO_RELOCK
	static constexpr uint32_t kAutoRelockTimeMs{ CONFIG_DOOR_LOCK_LOCK_SIM_AUTO_RELOCK_TIME_MS };
#endif // CONFIG_DOOR_LOCK_LOCK_SIM_AUTO_RELOCK

	static void ActuatorTimerEventHandler(k_timer *timer);
	static void NotifyWorkHandler(k_work *work);
#ifdef CONFIG_DOOR_LOCK_LOCK_SIM_AUTO_RELOCK
	static void AutoRelockTimerEventHandler(k_timer *timer);
#endif // CONFIG_DOOR_LOCK_LOCK_SIM_AUTO_RELOCK

	void StartOperation(OperationSource source, ReaderStateByte state);
	void ActuatorTimerEventHandler();
	void NotifyWorkHandler();
#ifdef CONFIG_DOOR_LOCK_LOCK_SIM_AUTO_RELOCK
	void StartAutoRelock();
	void AutoRelockTimerEventHandler();
#endif // CONFIG_DOOR_LOCK_LOCK_SIM_AUTO_RELOCK

	LockStateChangeCallback mLockStateChangeCallback{ nullptr };
	k_timer mActuatorTimer;
	k_work mNotifyWork;
	OperationSource mSource{ OperationSource::Unspecified };
	ReaderStateByte mState{ ReaderStateByte::Secured };
#ifdef CONFIG_DOOR_LOCK_LOCK_SIM_AUTO_RELOCK
	k_timer mAutoRelockTimer;
#endif // CONFIG_DOOR_LOCK_LOCK_SIM_AUTO_RELOCK
};

} // namespace Aliro
