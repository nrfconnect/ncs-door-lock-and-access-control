/*
 * SPDX-FileCopyrightText: Copyright (c) 2025 Qorvo, Inc.
 * SPDX-License-Identifier: LicenseRef-Qorvo-SDK-1
 */

#pragma once

#include <cherry/cherry.h>
#include <cherry/cherry_session.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * DOC: struct cherry_radar_session
 *
 * Object associated to a radar session.
 *
 * All sessions are associated with a session object, which is given to every
 * functions working with a session. It contains the session context.
 */
struct cherry_radar_session;

/**
 * struct cherry_radar_capabilities - Device capability parameters.
 */
struct cherry_radar_capabilities {
	/**
	 * @support: Support flag for radar feature.
	 *
	 * - b0 = boolean value.
	 */
	uint8_t support;
};

/**
 * enum cherry_radar_event_type - Type of event for radar session.
 */
enum cherry_radar_event_type {
	/**
	 * @CHERRY_RADAR_EVENT_TYPE_SESSION_STATUS: Radar session state has
	 * changed.
	 */
	CHERRY_RADAR_EVENT_TYPE_SESSION_STATUS,
	/**
	 * @CHERRY_RADAR_EVENT_TYPE_SESSION_ERROR: An error occurred with a
	 * session.
	 */
	CHERRY_RADAR_EVENT_TYPE_SESSION_ERROR,
	/**
	 * @CHERRY_RADAR_EVENT_TYPE_SESSION_REPORT: Report of radar session data.
	 */
	CHERRY_RADAR_EVENT_TYPE_SESSION_REPORT,
	/**
	 * @CHERRY_RADAR_EVENT_TYPE_SESSION_DIAGNOSTIC_REPORT: Report of radar diagnostic.
	 */
	CHERRY_RADAR_EVENT_TYPE_SESSION_DIAGNOSTIC_REPORT,
};

/**
 * enum cherry_radar_session_state - State of a radar session
 */
enum cherry_radar_session_state {
	/**
	 * @CHERRY_RADAR_SESSION_STATE_INIT: The session is created but not yet
	 * ready to be started.
	 */
	CHERRY_RADAR_SESSION_STATE_INIT,
	/**
	 * @CHERRY_RADAR_SESSION_STATE_DEINIT: The session is destroyed, it can
	 * no longer be used.
	 */
	CHERRY_RADAR_SESSION_STATE_DEINIT,
	/**
	 * @CHERRY_RADAR_SESSION_STATE_ACTIVE: The session is started.
	 */
	CHERRY_RADAR_SESSION_STATE_ACTIVE,
	/**
	 * @CHERRY_RADAR_SESSION_STATE_IDLE: The session is ready to be started
	 * or stopped. It can be stopped either because the max number of
	 * measurements as been reached, or because the session stop was
	 * requested.
	 */
	CHERRY_RADAR_SESSION_STATE_IDLE,
};

/**
 * enum cherry_radar_state_change_reason - Reason why the session state has
 * changed
 */
enum cherry_radar_state_change_reason {
	/**
	 * @CHERRY_RADAR_STATE_CHANGE_REASON_MGMT_CMD: Due to a session
	 * management command
	 */
	CHERRY_RADAR_STATE_CHANGE_REASON_MGMT_CMD,
	/**
	 * @CHERRY_RADAR_STATE_CHANGE_REASON_MAX_MEASUREMENT:
	 * The number of burst has reached the max value
	 * defined with cherry_radar_session_set_number_of_bursts()
	 */
	CHERRY_RADAR_STATE_CHANGE_REASON_MAX_MEASUREMENT,
	/**
	 * @CHERRY_RADAR_STATE_CHANGE_REASON_UNKNOWN: The reason is unknown
	 */
	CHERRY_RADAR_STATE_CHANGE_REASON_UNKNOWN,
	/**
	 * @CHERRY_RADAR_STATE_CHANGE_REASON_FORCE_STOPPED: The session
	 * has been force stopped
	 */
	CHERRY_RADAR_STATE_CHANGE_REASON_FORCE_STOPPED,
};

/**
 * struct cherry_radar_session_event_session_status - Data format for the
 * CHERRY_RADAR_EVENT_TYPE_SESSION_STATUS event.
 */
struct cherry_radar_session_event_session_status {
	/**
	 * @session: Session context.
	 */
	struct cherry_radar_session *session;
	/**
	 * @session_state: New state of the session.
	 */
	enum cherry_radar_session_state session_state;
	/**
	 * @reason_code: Reason for the session state change
	 */
	enum cherry_radar_state_change_reason reason_code;
};

/**
 * struct cherry_radar_session_event_error - Data format for the
 * CHERRY_RADAR_EVENT_TYPE_SESSION_ERROR event.
 */
struct cherry_radar_session_event_error {
	/**
	 * @session: Session context.
	 */
	struct cherry_radar_session *session;
	/**
	 * @status_err: Error code, see the definition of cherry_err.
	 */
	enum cherry_err status_err;
};

/**
 * struct data_fragment - Data fragments of the sweep.
 */
struct data_fragment {
	/**
	 * @size: Size of the data fragment in bytes.
	 */
	uint16_t size;
	/**
	 * @data: Pointer to the sweep data fragment.
	 */
	uint8_t *data;
};

/**
 * struct cherry_radar_sweep - Data format for the sweep
 * One sweep corresponding to the radar measurement for one burst.
 * A sweep contains multiple samples. Sweep is also known as CIR and Samples as
 * taps.
 */
struct cherry_radar_sweep {
	/**
	 * @sequence_number: Counter of the radar sweep.
	 * Starting from 0 when Radar session is started. Used to indicate
	 * missing measurements.
	 */
	uint32_t sequence_number;
	/**
	 * @timestamp: Timestamp of the sweep in RSTU.
	 */
	uint32_t timestamp;
	/**
	 * @vendor_data_len: Length of vendor data.
	 */
	uint32_t vendor_data_len;
	/**
	 * @vendor_data: Pointer to vendor data.
	 */
	uint8_t *vendor_data;
	/**
	 * @n_data_fragments: Number of data fragments.
	 */
	uint8_t n_data_fragments;
	/**
	 * @data_fragments: Data fragments of the sweep.
	 */
	struct data_fragment *data_fragments;
};

/**
 * enum cherry_radar_sample_size - Size of sample
 * @CHERRY_RADAR_SAMPLE_SIZE_32_BITS: 32 bits
 * @CHERRY_RADAR_SAMPLE_SIZE_48_BITS: 48 bits
 * @CHERRY_RADAR_SAMPLE_SIZE_64_BITS: 64 bits
 */
enum cherry_radar_sample_size {
	CHERRY_RADAR_SAMPLE_SIZE_32_BITS = 0x00,
	CHERRY_RADAR_SAMPLE_SIZE_48_BITS = 0x01,
	CHERRY_RADAR_SAMPLE_SIZE_64_BITS = 0x02
};

/**
 * enum cherry_radar_report_status - Radar session report status
 * @CHERRY_RADAR_REPORT_STATUS_SUCCESS: Success
 * @CHERRY_RADAR_REPORT_STATUS_ERROR: Error
 */
enum cherry_radar_report_status {
	CHERRY_RADAR_REPORT_STATUS_SUCCESS = 0x00,
	CHERRY_RADAR_REPORT_STATUS_ERROR = 0x01,
};

/**
 * struct cherry_radar_session_report - Data format for
 * CHERRY_RADAR_EVENT_TYPE_SESSION_REPORT event.
 */
struct cherry_radar_session_report {
	/**
	 * @session: Session context.
	 */
	struct cherry_radar_session *session;
	/**
	 * @status: Radar report status with:
	 *  - 0x00: Success
	 *  - 0x01: Error
	 */
	enum cherry_radar_report_status status;
	/**
	 * @n_sweeps: Number of sweeps stored in the sweeps table.
	 */
	uint8_t n_sweeps;
	/**
	 * @samples_per_sweep: Number of samples per sweep.
	 */
	uint8_t samples_per_sweep;
	/**
	 * @sample_size: size in bits of the samples.
	 */
	enum cherry_radar_sample_size sample_size;
	/**
	 * @sweeps: Radar measurements information.
	 * Size of the table is @n_sweeps.
	 */
	struct cherry_radar_sweep *sweeps;
};

/**
 * struct cherry_radar_event - Cherry radar event message.
 */
struct cherry_radar_event {
	/**
	 * @type: Event that occurred.
	 */
	enum cherry_radar_event_type type;
	/**
	 * @data: Pointer to the event data. Each event as it own data format
	 * and size.
	 */
	union {
		/**
		 * @data.status: data for the
		 * CHERRY_RADAR_EVENT_TYPE_SESSION_STATUS event.
		 */
		struct cherry_radar_session_event_session_status *status;
		/**
		 * @data.error: data for the
		 * CHERRY_RADAR_EVENT_TYPE_SESSION_ERROR event.
		 */
		struct cherry_radar_session_event_error *error;
		/**
		 * @data.report: data for the
		 * CHERRY_RADAR_EVENT_TYPE_SESSION_REPORT event.
		 */
		struct cherry_radar_session_report *report;
		/**
		 * @data.diagnostics: data for the
		 * CHERRY_RADAR_EVENT_TYPE_SESSION_DIAGNOSTIC_REPORT event.
		 */
		struct cherry_common_diag_report *diagnostics;
	} data;
};

/**
 * typedef cherry_radar_cb_t - Type for Cherry radar notification callback.
 * @event: The event type and data.
 * @user_data: Pointer to client data defined at the session creation.
 *
 * The event is allocated by cherry and has to be freed by the application
 * using cherry_radar_event_free().
 *
 * The callback implementation should return immediately and should not be
 * blocking.
 */
typedef void (*cherry_radar_cb_t)(struct cherry_radar_event *event,
				  void *user_data);

/**
 * cherry_radar_session_create() - Create and configure a Radar session.
 * @ctx: Cherry context.
 * @callback: Callback function to be call for session events.
 * @user_data: Optional pointer to data element managed by the Cherry's client.
 *             It is the pointer that will be put back as argument of radar
 *             event callback.
 * @session_id: Session identifier, any non-zero value.
 * @burst_period_ms: Interval between bursts, in milliseconds.
 * @sweep_period_rstu: Interval between sweeps inside a burst, in RSTU with
 *                     1200 RSTU equal to 1ms.
 * @sweeps_per_burst: Number of sweeps per burst.
 * @samples_per_sweep: Number of samples per sweep.
 * @antenna_set: Antenna set to use for transmit and receive path.
 *
 * This function allocates the session context which must be released using
 * cherry_radar_session_destroy(). Then it setups every needed parameters to
 * prepare the radar session.
 *
 * The sweep and burts parameters has to be consistent:
 *   @burst_period_ms >= @sweep_period_rstu/1200 * @sweeps_per_burst
 *
 * If the rule above is not respected, then the session will report an error
 * event at start.
 *
 * This function returns immediately.
 *
 * Returns: Newly allocated session or NULL on error.
 */
struct cherry_radar_session *cherry_radar_session_create(
	struct cherry *ctx, cherry_radar_cb_t callback, void *user_data,
	uint32_t session_id, uint32_t burst_period_ms,
	uint16_t sweep_period_rstu, uint8_t sweeps_per_burst,
	uint8_t samples_per_sweep, uint8_t antenna_set);

/**
 * cherry_radar_session_to_base() - Return the base session object
 * associated to a Radar one.
 * @session: FiRa session object.
 *
 * Returns: Generic session object.
 */
struct cherry_session *
cherry_radar_session_to_base(struct cherry_radar_session *session);

/**
 * cherry_radar_session_get_user_data() - Get the user_data of the session
 * @session: Session object.
 *
 * Returns: The user_data pointer given at the session creation.
 */
static inline void *
cherry_radar_session_get_user_data(struct cherry_radar_session *session)
{
	return cherry_session_get_user_data(
		cherry_radar_session_to_base(session));
}

/**
 * cherry_radar_event_free() - Release the event allocated memory.
 * @event: Pointer to the event to be freed.
 *
 * After each call to @cherry_radar_cb_t, this function has to be used to free
 * the corresponding event data. It let the upper layer decides either to fully
 * handle the event inside the callback or to deferred some processing outside
 * without temporary data copy.
 */
void cherry_radar_event_free(struct cherry_radar_event *event);

/**
 * cherry_radar_session_destroy() - Release a Radar session.
 * @session: Session object.
 *
 * This can be used on any session, active or not. The session will be stopped
 * if needed and removed from the UWB subsystem. The session is not destroyed
 * immediately, it is marked as destructed and can no longer be used by the API
 * client. The session context will be free after being removed from the UWB
 * subsystem.
 *
 * The session object must not be used after this call.
 *
 * This function returns immediately.
 */
static inline void
cherry_radar_session_destroy(struct cherry_radar_session *session)
{
	cherry_session_destroy(cherry_radar_session_to_base(session));
}

/**
 * cherry_radar_session_start() - Start a Radar session.
 * @session: Session object.
 *
 * Request a session to be started. This is done asynchronously and the session
 * state callback will be called once the session changes state, or if there is
 * an error.
 *
 * While a session is active, the report notification callback is called
 * after each burst to report the measurements.
 *
 * This function returns immediately.
 *
 * Returns:
 *  - &CHERRY_ERR_NONE if accepted
 */
static inline enum cherry_err
cherry_radar_session_start(struct cherry_radar_session *session)
{
	return cherry_session_start(cherry_radar_session_to_base(session));
}

/**
 * cherry_radar_session_stop() - Stop a session.
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
static inline enum cherry_err
cherry_radar_session_stop(struct cherry_radar_session *session)
{
	return cherry_session_stop(cherry_radar_session_to_base(session));
}

/**
 * DOC: Extra APIs for fine session configuration
 */

/**
 * cherry_radar_session_set_channel() - Set the RF channel for a session.
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
static inline enum cherry_err
cherry_radar_session_set_channel(struct cherry_radar_session *session,
				 int channel)
{
	return cherry_session_set_channel(cherry_radar_session_to_base(session),
					  channel);
}

/**
 * cherry_radar_session_set_number_of_bursts() - Set the number of measurement
 * before stopping the session
 * @session: Session object.
 * @max_burst: Max burst before stopping the session.
 *
 * When the max number of burst is set to 0, the session will not stop until the
 * cherry_radar_session_stop() is called. Otherwise it will automatically stop
 * when it reached the number of measurements requested.
 *
 * By default, if this function is not called, the value is set to 0.
 *
 * This function may be called at any time, as long as the session is not
 * active.
 *
 * This function returns immediately.
 *
 * returns:
 *  - &CHERRY_ERR_NONE if accepted
 *  - &CHERRY_ERR_INVALID_PARAMETER on any invalid parameter
 */
enum cherry_err
cherry_radar_session_set_number_of_bursts(struct cherry_radar_session *session,
					  uint16_t max_burst);

/**
 * cherry_radar_session_set_sweep_offset() - Set samples offset
 * @session: Session object.
 * @offset: offset of acquired CIR samples
 *
 * Number of samples offset before First Path. Possible values
 * are {-32768 to 32767} (Default = -10)
 *
 * This function may be called at any time, as long as the session is not
 * active.
 *
 * This function returns immediately.
 *
 * returns:
 *  - &CHERRY_ERR_NONE if accepted
 *  - &CHERRY_ERR_INVALID_PARAMETER on any invalid parameter
 */
enum cherry_err
cherry_radar_session_set_sweep_offset(struct cherry_radar_session *session,
				      int16_t offset);

/**
 * cherry_radar_session_set_tx_profile_idx() - Set the tx_profile_idx
 * @session: Session object.
 * @idx: index of tx power profile.
 * 0x00 = TX_PROFILE_HIGH (Default): TX profile for High radar range
 * 0x01 = TX_PROFILE_LOW: TX profile for Low radar range
 *
 * Setting parameters of transmission power of the signal to support
 * short/mid/long range radar.
 * profiles numbers are used directly as an indexes for the lut_tx_profile so
 * they need to be continues, non-decreasing and starting from 0
 *
 * This function may be called at any time, as long as the session is not
 * active.
 *
 * This function returns immediately.
 *
 * returns:
 *  - &CHERRY_ERR_NONE if accepted
 *  - &CHERRY_ERR_INVALID_PARAMETER on any invalid parameter
 */
enum cherry_err
cherry_radar_session_set_tx_profile_idx(struct cherry_radar_session *session,
					uint8_t idx);

/**
 * cherry_radar_session_set_preamble_duration() - Set the preamble duration for a session.
 * @session: Session object.
 * @preamble_duration: Preamble Symbol Repetitions (PSR).
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
enum cherry_err cherry_radar_session_set_preamble_duration(
	struct cherry_radar_session *session,
	enum cherry_common_preamble_duration preamble_duration);

/**
 * cherry_radar_session_set_rframe_config() - Set the rframe config for a session.
 * @session: Session object.
 * @rframe_config: STS Packet Configuration.
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
enum cherry_err cherry_radar_session_set_rframe_config(
	struct cherry_radar_session *session,
	enum cherry_common_rframe_config rframe_config);

/**
 * cherry_radar_session_set_preamble_code_index() - Set the preamble code index for a session.
 * @session: Session object.
 * @preamble_code_index: preamble_code_index.
 *
 * This function may be called at any time, as long as the session is not
 * active. It does not have to be called if no change is needed from default
 * parameters.
 * By default, if this function is not called, the value is set to 10.
 * Set the preamble code index >= 25 configures as well the FW to use HPRF.
 *
 * This function returns immediately.
 *
 * Returns:
 *  - &CHERRY_ERR_NONE if accepted
 *  - &CHERRY_ERR_INVALID_PARAMETER on any invalid parameter
 */
static inline enum cherry_err cherry_radar_session_set_preamble_code_index(
	struct cherry_radar_session *session, uint8_t preamble_code_index)
{
	return cherry_session_set_preamble_code_index(
		cherry_radar_session_to_base(session), preamble_code_index);
}

/**
 * cherry_radar_session_set_diagnostics() - Enable or disable session
 * diagnostics.
 * @session: Session object.
 * @config: Define which kind of diagnostics to enable.
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
static inline enum cherry_err
cherry_radar_session_set_diagnostics(struct cherry_radar_session *session,
				     struct cherry_common_diag_cfg config)
{
	return cherry_session_set_diagnostics(
		cherry_radar_session_to_base(session), config, false);
}

#ifdef __cplusplus
}
#endif
