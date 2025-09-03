/*
 * SPDX-FileCopyrightText: Copyright (c) 2024 Qorvo, Inc.
 * SPDX-License-Identifier: LicenseRef-Qorvo-SDK-1
 */

#pragma once

#include <qerr.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * enum qpm_sleep_state - QOSAL Power sleep state, following ACPI standard.
 * @QPM_STATE_S0: ACPI Sleep State S0.
 * @QPM_STATE_S0ix: ACPI Sleep State S0ix.
 * @QPM_STATE_S1: ACPI Sleep State S1.
 * @QPM_STATE_S2: ACPI Sleep State S2.
 * @QPM_STATE_S3: ACPI Sleep State S3.
 * @QPM_STATE_S4: ACPI Sleep State S4.
 * @QPM_STATE_S5: ACPI Sleep State S5.
 */
enum qpm_sleep_state {
	QPM_STATE_S0,
	QPM_STATE_S0ix,
	QPM_STATE_S1,
	QPM_STATE_S2,
	QPM_STATE_S3,
	QPM_STATE_S4,
	QPM_STATE_S5,
};

/**
 * QPM_ALL_SUBSTATES - Select all sub-states of a given power sleep state.
 */
#define QPM_ALL_SUBSTATES 0xFFU

/**
 * qpm_sleep_state_lock() - Disallow a power sleep state by increasing a lock
 * counter.
 * @state: Power sleep state.
 * @substate_id: ID of sub-state.
 *
 * Lock power sleep states higher or equal the given sleep state. This means
 * that only the sleep states below the specified one will be accessible.
 *
 * For example ``qpm_mgmt_state_lock(QPM_STATE_S1, QPM_ALL_SUBSTATES)``
 * will allow the states QPM_STATE_S0 and QPM_STATE_S0ix only.
 *
 * To allow again QPM_STATE_S1 and higher, the sleep state must be unlocked the
 * same number of times it has been locked, so that the lock counter equals 0.
 */
void qpm_sleep_state_lock(enum qpm_sleep_state state, uint8_t substate_id);

/**
 * qpm_sleep_state_unlock() - Allow a power sleep state by decreasing a lock
 * counter.
 * @state: Power sleep state.
 * @substate_id: ID of sub-state.
 */
void qpm_sleep_state_unlock(enum qpm_sleep_state state, uint8_t substate_id);

/**
 * qpm_sleep_state_is_active() - Check if a power sleep state is active
 * (unlocked) or not.
 * @state: Power sleep state.
 * @substate_id: ID of sub-state.
 *
 * In order to be active, the lock counter of a power sleep state must equals 0.
 *
 * Return: True if power sleep state is unlocked, false otherwise.
 */
bool qpm_sleep_state_is_active(enum qpm_sleep_state state, uint8_t substate_id);

/**
 * qpm_set_low_power_mode() - Set low power mode state.
 * @enabled: Enable or disable low power mode state.
 *
 * With current implementation, qpm_set_low_power_mode() must be called
 * at least once before using qpm_get_low_power_mode(). Is ensures
 * persited low power config matches actual setting.
 *
 * Return: QERR_SUCCESS or error.
 */
enum qerr qpm_set_low_power_mode(bool enabled);

/**
 * qpm_get_low_power_mode() - get low power mode state.
 *
 * Return: True if low power mode is set, otherwise false.
 */
bool qpm_get_low_power_mode(void);

/**
 * qpm_set_min_inactivity_s4() - Set the minimum inactivity time to enter S4.
 *
 * @time_ms: minimum inactivity time to get in S4, in ms.
 *
 * Return: QERR_SUCCESS or error.
 */
enum qerr qpm_set_min_inactivity_s4(uint32_t time_ms);

/**
 * qpm_get_min_inactivity_s4() - Get the minimum inactivity time to enter S4.
 *
 * @time_ms: minimum inactivity time to get in S4, in ms.
 *
 * Return: QERR_SUCCESS or error.
 */
enum qerr qpm_get_min_inactivity_s4(uint32_t *time_ms);

#ifdef __cplusplus
}
#endif
