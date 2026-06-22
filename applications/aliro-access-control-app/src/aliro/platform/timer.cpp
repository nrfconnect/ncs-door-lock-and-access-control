/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

#include "timer.h"

#include <aliro/utils.h>
#include <aliro_workqueue/aliro_workqueue.h>

#include <tuple>

namespace Aliro {

Timer::Timer(uint32_t timeoutMs, Callback callback, Context userData)
	: mTimeout{ K_MSEC(timeoutMs) }, mCallback{ callback }, mContext{ userData }
{
	k_timer_init(
		&mTimer,
		[](k_timer *timer) {
			auto timerObj = static_cast<Timer *>(k_timer_user_data_get(timer));
			VerifyOrDie(timerObj, "Invalid timer");

			std::ignore = AliroWorkqueueSubmit(&timerObj->mWork);
		},
		nullptr);

	k_timer_user_data_set(&mTimer, this);

	k_work_init(&mWork, [](k_work *work) {
		auto *timer = CONTAINER_OF(work, Timer, mWork);
		timer->mCallback(timer->mContext);
	});
}

Timer::~Timer()
{
	k_timer_stop(&mTimer);

	// wait for the work to be cancelled before destroying the timer
	k_work_sync sync{};
	k_work_cancel_sync(&mWork, &sync);
}

void Timer::Start()
{
	k_timer_start(&mTimer, mTimeout, K_NO_WAIT);
}

void Timer::Restart()
{
	Start();
}

bool Timer::IsRunning()
{
	return k_timer_remaining_ticks(&mTimer) != 0;
}

void Timer::Stop()
{
	k_timer_stop(&mTimer);
}

} // namespace Aliro
