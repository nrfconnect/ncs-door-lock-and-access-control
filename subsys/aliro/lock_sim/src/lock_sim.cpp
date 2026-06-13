/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "lock_sim.h"

#include <aliro/utils.h>
#include <aliro_workqueue/aliro_workqueue.h>

#ifdef CONFIG_DOOR_LOCK_ALIRO_LOCK_SIM_INDICATOR
#include <zephyr/drivers/gpio.h>
#endif // CONFIG_DOOR_LOCK_ALIRO_LOCK_SIM_INDICATOR

#include <zephyr/logging/log.h>

#include <tuple>

LOG_MODULE_REGISTER(lock_sim, CONFIG_DOOR_LOCK_APP_LOG_LEVEL);

namespace Aliro {

namespace {

#ifdef CONFIG_DOOR_LOCK_ALIRO_LOCK_SIM_INDICATOR

constexpr int kLedOn{ 1 };
constexpr int kLedOff{ 0 };

constexpr gpio_dt_spec kLockSimIndicatorLed = GPIO_DT_SPEC_GET(DT_ALIAS(lock_sim_indicator), gpios);

#endif // CONFIG_DOOR_LOCK_ALIRO_LOCK_SIM_INDICATOR

} // namespace

void LockSim::Init(LockStateChangeCallback callback)
{
	mLockStateChangeCallback = callback;

	k_timer_init(&mActuatorTimer, &LockSim::ActuatorTimerEventHandler, nullptr);
	k_timer_user_data_set(&mActuatorTimer, this);

	k_work_init(&mNotifyWork, &LockSim::NotifyWorkHandler);

#ifdef CONFIG_DOOR_LOCK_ALIRO_LOCK_SIM_AUTO_RELOCK
	k_timer_init(&mAutoRelockTimer, &LockSim::AutoRelockTimerEventHandler, nullptr);
	k_timer_user_data_set(&mAutoRelockTimer, this);
#endif // CONFIG_DOOR_LOCK_ALIRO_LOCK_SIM_AUTO_RELOCK

#ifdef CONFIG_DOOR_LOCK_ALIRO_LOCK_SIM_INDICATOR
	VerifyOrDie(gpio_is_ready_dt(&kLockSimIndicatorLed), "Lock simulator indicator GPIO not ready");
	VerifyOrDie(gpio_pin_configure_dt(&kLockSimIndicatorLed, GPIO_OUTPUT_INACTIVE) == 0,
		    "Failed to configure lock simulator indicator GPIO");
#endif // CONFIG_DOOR_LOCK_ALIRO_LOCK_SIM_INDICATOR
}

bool LockSim::Lock(OperationSource source)
{
	VerifyOrReturnFalse(mState != ReaderStateByte::Secured);
	StartOperation(source, ReaderStateByte::EnteringSecured);
	return true;
}

bool LockSim::Unlock(OperationSource source)
{
	VerifyOrReturnFalse(mState != ReaderStateByte::Unsecured);
	StartOperation(source, ReaderStateByte::EnteringUnsecured);
	return true;
}

void LockSim::StartOperation(OperationSource source, ReaderStateByte state)
{
	mSource = source;
	mState = state;

	std::ignore = AliroWorkqueueSubmit(&mNotifyWork);
	k_timer_start(&mActuatorTimer, K_MSEC(kActuatorMovementTimeMs), K_NO_WAIT);
}

void LockSim::ActuatorTimerEventHandler(k_timer *timer)
{
	LockSim *lockSim = static_cast<LockSim *>(k_timer_user_data_get(timer));
	VerifyOrDie(lockSim, "Invalid LockSim instance");

	lockSim->ActuatorTimerEventHandler();
}

void LockSim::ActuatorTimerEventHandler()
{
	const ReaderStateByte prevState{ mState };

	switch (mState) {
	case ReaderStateByte::EnteringSecured:
		mState = ReaderStateByte::Secured;
		break;
	case ReaderStateByte::EnteringUnsecured:
		mState = ReaderStateByte::Unsecured;
		break;
	default:
		break;
	}

	if (prevState != mState) {
		std::ignore = AliroWorkqueueSubmit(&mNotifyWork);
	}
}

void LockSim::NotifyWorkHandler(k_work *work)
{
	auto *lockSim = CONTAINER_OF(work, LockSim, mNotifyWork);
	lockSim->NotifyWorkHandler();
}

void LockSim::NotifyWorkHandler()
{
	switch (mState) {
	case ReaderStateByte::Secured:
		LOG_INF("Locking the lock completed");
#ifdef CONFIG_DOOR_LOCK_ALIRO_LOCK_SIM_INDICATOR
		gpio_pin_set_dt(&kLockSimIndicatorLed, kLedOff);
#endif // CONFIG_DOOR_LOCK_ALIRO_LOCK_SIM_INDICATOR
		break;
	case ReaderStateByte::Unsecured:
		LOG_INF("Unlocking the lock completed");
#ifdef CONFIG_DOOR_LOCK_ALIRO_LOCK_SIM_INDICATOR
		gpio_pin_set_dt(&kLockSimIndicatorLed, kLedOn);
#endif // CONFIG_DOOR_LOCK_ALIRO_LOCK_SIM_INDICATOR
#ifdef CONFIG_DOOR_LOCK_ALIRO_LOCK_SIM_AUTO_RELOCK
		StartAutoRelock();
#endif // CONFIG_DOOR_LOCK_ALIRO_LOCK_SIM_AUTO_RELOCK
		break;
	case ReaderStateByte::EnteringSecured:
		LOG_INF("Locking the lock initiated");
		break;
	case ReaderStateByte::EnteringUnsecured:
		LOG_INF("Unlocking the lock initiated");
		break;
	default:
		break;
	}

	VerifyAndCall(mLockStateChangeCallback, mSource, mState);
}

#ifdef CONFIG_DOOR_LOCK_ALIRO_LOCK_SIM_AUTO_RELOCK

void LockSim::StartAutoRelock()
{
	k_timer_start(&mAutoRelockTimer, K_MSEC(kAutoRelockTimeMs), K_NO_WAIT);
}

void LockSim::AutoRelockTimerEventHandler(k_timer *timer)
{
	auto *lockSim = static_cast<LockSim *>(k_timer_user_data_get(timer));
	VerifyOrDie(lockSim, "Invalid LockSim instance");

	lockSim->AutoRelockTimerEventHandler();
}

void LockSim::AutoRelockTimerEventHandler()
{
	if (mState == ReaderStateByte::Unsecured) {
		StartOperation(OperationSource::Auto, ReaderStateByte::EnteringSecured);
	}
}

#endif // CONFIG_DOOR_LOCK_ALIRO_LOCK_SIM_AUTO_RELOCK

} // namespace Aliro
