/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include <zephyr/kernel.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Submit a work item to the dedicated Aliro workqueue.
 *
 * @param work Work item to submit.
 *
 * @return as with k_work_submit_to_queue().
 */
int AliroWorkqueueSubmit(struct k_work *work);

/**
 * @brief Reschedule a delayable work item on the dedicated Aliro workqueue.
 *
 * @param work Delayable work item to schedule.
 * @param delay Delay before the work should run.
 *
 * @return as with k_work_reschedule_for_queue().
 */
int AliroWorkqueueReschedule(struct k_work_delayable *work, k_timeout_t delay);

#ifdef __cplusplus
}
#endif
