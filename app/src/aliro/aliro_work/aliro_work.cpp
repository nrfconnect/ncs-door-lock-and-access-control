/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "aliro_work.h"

#include <zephyr/kernel.h>

namespace {

k_work_q sAliroWorkQ;
K_THREAD_STACK_DEFINE(sAliroWorkQStack, CONFIG_DOOR_LOCK_ALIRO_WORKQUEUE_STACK_SIZE);
bool sAliroWorkQStarted = false;

} // namespace

extern "C" {

int AliroWorkSubmit(struct k_work *work)
{
	return k_work_submit_to_queue(&sAliroWorkQ, work);
}

int AliroWorkReschedule(struct k_work_delayable *work, k_timeout_t delay)
{
	return k_work_reschedule_for_queue(&sAliroWorkQ, work, delay);
}

int AliroWorkInit(void)
{
	if (sAliroWorkQStarted) {
		return 0;
	}

	k_work_queue_start(&sAliroWorkQ, sAliroWorkQStack, K_THREAD_STACK_SIZEOF(sAliroWorkQStack),
			   CONFIG_DOOR_LOCK_ALIRO_WORKQUEUE_PRIORITY, nullptr);
	sAliroWorkQStarted = true;
	return 0;
}

} // extern "C"
