/**
 * @file      qerr.h
 *
 * @brief     Header file for Qorvo workqueue
 *
 * @author    Qorvo Paris Applications
 *
 * @copyright SPDX-FileCopyrightText: Copyright (c) 2024 Qorvo, Inc.
 *            SPDX-License-Identifier: LicenseRef-Qorvo-SDK-1
 *
 */

#pragma once

#include <qerr.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * struct qworkqueue: QOSAL Work Queue (opaque)
 */
struct qworkqueue;

/**
 * typedef qwork_func - Pointer to a work task entry point.
 * @arg: private data of the workqueue.
 *
 * Return: nothing.
 *
 */
typedef void (*qwork_func)(void *arg);

/**
 * qworkqueue_init() - Initialize a workqueue.
 * @handler: Entry point of the work task.
 * @priv: Private data of the work task.
 *
 * Return: Pointer to the initialized workqueue.
 */
struct qworkqueue *qworkqueue_init(qwork_func handler, void *priv);

/**
 * qworkqueue_schedule_work() - Schedule work task in a workqueue.
 * @workqueue: Pointer to the workqueue.
 *
 * Return: QERR_SUCCESS or error.
 */
enum qerr qworkqueue_schedule_work(struct qworkqueue *workqueue);

/**
 * qworkqueue_cancel_work() - Cancel work task in a workqueue and free the
 * queue.
 * @workqueue: Pointer to the workqueue.
 *
 * Return: QERR_SUCCESS or error.
 */
enum qerr qworkqueue_cancel_work(struct qworkqueue *workqueue);

#ifdef __cplusplus
}
#endif
