/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <aliro/aliro.h>
#include <aliro/errors.h>
#include <aliro/interface.h>
#include <aliro/utils.h>
#include <aliro_workqueue/aliro_workqueue.h>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/atomic.h>

LOG_MODULE_REGISTER(interface_events, CONFIG_DOOR_LOCK_APP_LOG_LEVEL);

namespace Aliro::Interface::Os {

namespace {

K_FIFO_DEFINE(sEventsFifo);
/*
 * Set by producers after enqueue.
 * Worker clears it before draining and checks it again after drain:
 * if it was set meanwhile, new events arrived concurrently and the worker
 * should continue (or resubmit during exit window handling).
 */
atomic_t sNewEvents = ATOMIC_INIT(0);
/*
 * Scheduling/running token for the worker.
 * 0: no work item queued/running
 * 1: work item queued/running
 *
 * Producer uses CAS(0->1) to ensure only one submit for a burst.
 * Worker clears it at end, then performs a final resubmit check to close
 * the race where producer enqueued while worker was still active.
 */
atomic_t sWorkerActive = ATOMIC_INIT(0);

void ProcessEventsWorkHandler(k_work *work)
{
	while (true) {
		/*
		 * Start a new drain pass. Any producer that enqueues during/after this
		 * point will set sNewEvents again, which we observe after FIFO becomes
		 * empty to decide whether another pass is needed.
		 */
		atomic_clear(&sNewEvents);

		while (void *event = k_fifo_get(&sEventsFifo, K_NO_WAIT)) {
			AliroStack::Instance().ProcessEvent(event);
		}

		if (!atomic_get(&sNewEvents)) {
			break;
		}
	}

	atomic_clear(&sWorkerActive);

	/*
	 * Close the exit race:
	 * - producer may enqueue just before/around worker deactivation
	 * - producer cannot submit while sWorkerActive==1
	 * - therefore worker performs one final check and self-resubmits if needed
	 */
	if (atomic_get(&sNewEvents) && atomic_cas(&sWorkerActive, 0, 1)) {
		const int submitErr = AliroWorkqueueSubmit(work);
		if (submitErr < 0) {
			atomic_clear(&sWorkerActive);
			LOG_ERR("Failed to resubmit events work, err: %d", submitErr);
		}
	}
}

K_WORK_DEFINE(sProcessEventsWork, ProcessEventsWorkHandler);

} // namespace

AliroError QueueEvent(void *event)
{
	VerifyOrReturnStatus(event != nullptr, ALIRO_INVALID_ARGUMENT);

	k_fifo_put(&sEventsFifo, event);
	/* Signal that at least one event is pending since current worker pass started. */
	atomic_set(&sNewEvents, 1);

	/* First producer in a burst schedules processing; others only enqueue+mark. */
	if (atomic_cas(&sWorkerActive, 0, 1)) {
		const int submitErr = AliroWorkqueueSubmit(&sProcessEventsWork);
		if (submitErr < 0) {
			atomic_clear(&sWorkerActive);
			/* Do not return here, as it may cause the stack to free the event
			 * before removing it from the queue. */
		}
	}

	return ALIRO_NO_ERROR;
}

} // namespace Aliro::Interface::Os
