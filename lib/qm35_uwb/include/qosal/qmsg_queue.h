/*
 * SPDX-FileCopyrightText: Copyright (c) 2024 Qorvo, Inc.
 * SPDX-License-Identifier: LicenseRef-Qorvo-SDK-1
 */

#pragma once

#include "qtime.h"

#include <qerr.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * struct qmsg_queue: QOSAL Message Queue (opaque).
 */
struct qmsg_queue;

/**
 * qmsg_queue_init() - Initialize a message queue.
 * @msg_queue_buffer: Message queue buffer.
 * @item_size: Size of the items in the message queue.
 * @max_item: Maximum number of items in the message queue.
 *
 * NOTE:
 *  1. If msg_queue_buffer is NULL, it will be automatically allocated of (max_item * item_size).
 *  2. Message queue thread-safety is not guaranteed and is implementation-defined.
 *
 * Return: Pointer to the initialized message queue.
 */
struct qmsg_queue *qmsg_queue_init(char *msg_queue_buffer, uint32_t item_size, uint32_t max_item);

/**
 * qmsg_queue_deinit() - De-initialize a message queue.
 * @msg_queue: Pointer to the message queue.
 */
void qmsg_queue_deinit(struct qmsg_queue *msg_queue);

/**
 * qmsg_queue_put() - Push an item in the message queue.
 * @msg_queue: Pointer to the message queue.
 * @item: Item to push into the queue.
 *
 * Return: QERR_SUCCESS on success, else another enum qerr value.
 */
enum qerr qmsg_queue_put(struct qmsg_queue *msg_queue, const void *item);

/**
 * qmsg_queue_get() - Get an item from the message queue.
 * @msg_queue: Pointer to the message queue.
 * @item: Pointer to a buffer that will get the first item of the queue or NULL
 * if the queue is empty.
 * @timeout_ms: Delay until timeout in ms. Use `QOSAL_WAIT_FOREVER` to wait
 * indefinitely.
 *
 * Return: QERR_SUCCESS or error.
 */
enum qerr qmsg_queue_get(struct qmsg_queue *msg_queue, void *item, uint32_t timeout_ms);

#ifdef __cplusplus
}
#endif
