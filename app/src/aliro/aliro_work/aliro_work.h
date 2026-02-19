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
 * @brief Initialize and start the dedicated Aliro workqueue.
 *
 * @return 0 on success.
 */
int AliroWorkInit(void);

/**
 * @brief Submit a work item to the dedicated Aliro workqueue.
 *
 * @param work Work item to submit.
 *
 * @return Return 0 on success, negative errno on failure.
 */
int AliroWorkSubmit(struct k_work *work);

/**
 * @brief Reschedule a delayable work item on the dedicated Aliro workqueue.
 *
 * @param work Delayable work item to schedule.
 * @param delay Delay before the work should run.
 *
 * @return Return 0 on success, negative errno on failure.
 */
int AliroWorkReschedule(struct k_work_delayable *work, k_timeout_t delay);

#ifdef __cplusplus
}
#endif
