/**
 * @file      qsignal.h
 *
 * @brief     Header file for Qorvo signal management
 *
 * @author    Qorvo Paris Applications
 *
 * @copyright SPDX-FileCopyrightText: Copyright (c) 2024 Qorvo, Inc.
 *            SPDX-License-Identifier: LicenseRef-Qorvo-SDK-1
 *
 */

#pragma once

#include <qerr.h>
#include <qtime.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * struct qsignal: QOSAL Signal (opaque).
 */
struct qsignal;

/**
 * qsignal_init() - Initialize a signal.
 *
 * NOTE: Signal thread-safety is not guaranteed and is implementation-defined.
 *
 * Return: Pointer to the initialized signal.
 */
struct qsignal *qsignal_init(void);

/**
 * qsignal_deinit() - De-initialize a signal.
 * @signal: Pointer to the signal.
 */
void qsignal_deinit(struct qsignal *signal);

/**
 * qsignal_raise() - Raise a signal.
 * @signal: Pointer to the signal.
 * @value: The value sent by the signal.
 *
 * Return: QERR_SUCCESS or error.
 */
enum qerr qsignal_raise(struct qsignal *signal, int value);

/**
 * qsignal_wait() - Wait for a signal.
 * @signal: Pointer to the signal.
 * @value: Pointer that will be filled with the value of the signal.
 * @timeout_ms: Delay until timeout in ms.
 *
 * Return: QERR_SUCCESS or error.
 */
enum qerr qsignal_wait(struct qsignal *signal, int *value, uint32_t timeout_ms);

#ifdef __cplusplus
}
#endif
