/*
 * SPDX-FileCopyrightText: Copyright (c) 2025 Qorvo, Inc.
 * SPDX-License-Identifier: LicenseRef-Qorvo-SDK-1
 */

#pragma once

#include <cherry/cherry.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * DOC: struct cherry_session
 *
 * Generic session object.
 *
 * All sessions are associated with a session object which is given to every
 * functions working with a session.
 */
struct cherry_session;

/**
 * cherry_session_get_user_data() - Get the user_data of the session
 * @session: Session object.
 *
 * Returns: The user_data pointer given at the session creation.
 */
void *cherry_session_get_user_data(struct cherry_session *session);

/**
 * cherry_session_destroy() - Release a session.
 * @session: Session object.
 *
 * This can be used on any session, active or not. The session will be stopped
 * if needed and removed from the UWB subsystem. The session is not destroyed
 * immediately, it is marked as destructed and can no longer be used by the API
 * client. The session context will be freed after being removed from the UWB
 * subsystem.
 *
 * The session object must not be used after this call.
 *
 * This function returns immediately.
 */
void cherry_session_destroy(struct cherry_session *session);

/**
 * cherry_session_start() - Start a session.
 * @session: Session object.
 *
 * Request a session to be started. This is done asynchronously and the session
 * state callback will be called once the session changes state, or if there is
 * an error.
 *
 * While a session is active, the ranging notification callback is called
 * regularly when results are available.
 *
 * This function returns immediately.
 *
 * Returns:
 *  - &CHERRY_ERR_NONE if accepted
 *  - &CHERRY_ERR_SESSION_CONFIG if session is missing some key configuration
 */
enum cherry_err cherry_session_start(struct cherry_session *session);

/**
 * cherry_session_stop() - Stop a session.
 * @session: Session object.
 *
 * Request a session to be stopped. This is done asynchronously and the session
 * state callback will be called once the session changes state, or if there is
 * an error.
 *
 * This function returns immediately.
 *
 * Returns:
 *  - &CHERRY_ERR_NONE if accepted
 *  - &CHERRY_ERR_INVALID_PARAMETER on any invalid parameter
 */
enum cherry_err cherry_session_stop(struct cherry_session *session);

/* Extra APIs for fine session configuration */

/**
 * cherry_session_set_channel() - Sets the RF channel for a session.
 * @session: Session object.
 * @channel: Channel number.
 *
 * This function may be called at any time, as long as the session is not
 * active. It does not have to be called if no change is needed from default
 * parameters.
 *
 * This function returns immediately.
 *
 * Returns:
 *  - &CHERRY_ERR_NONE if accepted
 *  - &CHERRY_ERR_INVALID_PARAMETER on any invalid parameter
 */
enum cherry_err cherry_session_set_channel(struct cherry_session *session,
					   int channel);

/**
 * cherry_session_set_preamble_code_index() - Sets preamble code index for a
 * session.
 * @session: Session object.
 * @preamble_code_index: preamble_code_index.
 *
 * This function may be called at any time, as long as the session is not
 * active. It does not have to be called if no change is needed from default
 * parameters.
 * By default, if this function is not called, the value is set to 10.
 *
 * This function returns immediately.
 *
 * Returns:
 *  - &CHERRY_ERR_NONE if accepted
 *  - &CHERRY_ERR_INVALID_PARAMETER on any invalid parameter
 */
enum cherry_err
cherry_session_set_preamble_code_index(struct cherry_session *session,
				       uint8_t preamble_code_index);

/**
 * cherry_session_set_priority() - Sets the session priority for a session.
 * @session: Session object.
 * @session_priority: session_priority.
 *
 * This function may be called at any time, as long as the session is not
 * active. It does not have to be called if no change is needed from default
 * parameters.
 * By default, if this function is not called, the value is set to 50.
 *
 * This function returns immediately.
 *
 * Returns:
 *  - &CHERRY_ERR_NONE if accepted
 *  - &CHERRY_ERR_INVALID_PARAMETER on any invalid parameter
 */
enum cherry_err cherry_session_set_priority(struct cherry_session *session,
					    uint8_t session_priority);

/**
 * cherry_session_set_antennas() - Select the Antenna Set to be used for the session.
 * @session: Session object.
 * @tx_antenna_set: Antenna set to be used as Tx for the 1st ranging round.
 * @rx_antenna_set: Antenna set to be used as Rx for the 1st ranging round.
 *
 * By default, if this function is not called, the value is set to 0.
 *
 * This function may be called at any time, as long as the session is not
 * active.
 *
 * This function returns immediately.
 *
 * Returns:
 *  - &CHERRY_ERR_NONE if accepted
 *  - &CHERRY_ERR_INVALID_PARAMETER on any invalid parameter
 */
enum cherry_err cherry_session_set_antennas(struct cherry_session *session,
					    uint8_t tx_antenna_set,
					    uint8_t rx_antenna_set);

/**
 * cherry_session_set_ref_time_base() - Configure the session time reference.
 * @session: Session object.
 * @session_reference: Session object to be used as time reference.
 * @offset_us: Offset, in microseconds, between the @session_reference and
 *             @session round start.
 *
 * This function may be called at any time, as long as the session is not
 * active.
 *
 * It allows to synchronize the session start with another session.
 *
 * This function returns immediately.
 *
 * Returns:
 *  - &CHERRY_ERR_NONE if accepted
 *  - &CHERRY_ERR_INVALID_PARAMETER on any invalid parameter
 */
enum cherry_err
cherry_session_set_ref_time_base(struct cherry_session *session,
				 struct cherry_session *session_reference,
				 uint32_t offset_us);

/**
 * cherry_session_set_diagnostics() - Enable or disable session
 * diagnostics.
 * @session: Session object.
 * @config: Define which kind of diagnostics to enable.
 * @enable_fallback: If true, enable fallback mechanism if needed.
 *
 * This function may be called at any time, as long as the session is not
 * active. It does not have to be called if no change is needed from default
 * parameters.
 *
 * This function returns immediately.
 *
 * Returns:
 *  - &CHERRY_ERR_NONE if accepted
 *  - &CHERRY_ERR_INVALID_PARAMETER on any invalid parameter or not supported
 *    diagnostics
 */
enum cherry_err
cherry_session_set_diagnostics(struct cherry_session *session,
			       struct cherry_common_diag_cfg config,
			       bool enable_fallback);

#ifdef __cplusplus
}
#endif
