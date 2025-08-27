/*
 * Warning: The APIs listed in this header file are experimental and subject to change.
 * They may be removed in future releases without prior notice. Use with caution. They
 * should not be used in production code.
 *
 * SPDX-FileCopyrightText: Copyright (c) 2025 Qorvo, Inc.
 * SPDX-License-Identifier: LicenseRef-Qorvo-SDK-1
 */

#pragma once

#include <cherry/cherry.h>
#include <cherry/cherry_common.h>
#include <cherry/cherry_session.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CHERRY_CCC_HOP_MODE_KEY_SIZE 4

/**
 * DOC: struct cherry_ccc_session
 *
 * Object associated to a CCC session.
 *
 * All sessions are associated with a session object, which is given to every
 * functions working with a session. It contains the session context.
 */
struct cherry_ccc_session;

/**
 * struct cherry_ccc_capabilities - Device capability parameters.
 *
 * From SUS API and UCI Spec Support for CCC Digital Key Interface Requirements_r24.
 */
struct cherry_ccc_capabilities {
	/**
	 * @slot_bitmask: Bitmap of supported values of Slot durations as a multiple of TChap.
	 *
	 * Each bit corresponds to a specific value of NChap_per_Slot:
	 *
	 * - b0 = 3
	 * - b1 = 4
	 * - b2 = 6
	 * - b3 = 8
	 * - b4 = 9
	 * - b5 = 12
	 * - b6 = 24
	 * - b7 is reserved.
	 *
	 * Note: b2 is used for testing only.
	 */
	uint8_t slot_bitmask;
	/**
	 * @sync_code_index_bitmask: Bitmap of SYNC code indices that can be used.
	 *
	 * Bits 0 to 31 represents index values for SYNC
	 * code that can be used from 1 to 32.
	 */
	uint32_t sync_code_index_bitmask;
	/**
	 * @hopping_config_bitmask: Bitmap for supported hopping modes and sequences.
 	 *
	 * 	b0: AES based hopping sequence.
	 *	b1: Default hopping sequence (always set).
	 *	b2: Adaptative hopping mode.
	 *	b3: Continuous hopping mode (always set).
	 *	b4: No hopping.
	 *
	 *	Rest of the bits are reserved for future hopping sequences.
	 *	b5-b7 = RFUs.
	 */
	uint8_t hopping_config_bitmask;
	/**
	 * @channel_bitmask: Bitmap of supported UWB channels.
	 *
	 * Each bit corresponds to a specific value of UWB channel where:
	 *
	 * - b0 = channel 5.
	 * - b1 = channel 9.
	 */
	uint8_t channel_bitmask;
	/**
	 * @protocol_versions: A list of supported protocol versions.
	 *
	 * Digital Key Protocol Version 1.0 is coded as 0x0100.
	 * If a future protocol version 0x0101 is supported, the response
	 * should be: 0x0100, 0x0101.
	 */
	struct {
		/**
		 * @protocol_versions.len: Length of array, number of elements.
		 */
		size_t len;
		/**
		 * @protocol_versions.items: Pointer to items of array.
		 */
		uint16_t *items;
	} protocol_versions;
	/**
	 * @uwb_configs: A list of supported UWBS configurations.
	 *
	 * Configuration 0x00 is mandatory for device and vehicle, whereas
	 * configuration 0x01 is mandatory for the device and optional for the vehicle.
	 */
	struct {
		/**
		 * @uwb_configs.len: Length of array, number of elements.
		 */
		size_t len;
		/**
		 * @uwb_configs.items: Pointer to items of array.
		 */
		uint16_t *items;
	} uwb_configs;
	/**
	 * @pulse_shape_combos: A list of supported PulseShape combinations.
	 *
	 * b3-b0: Initiator transmit pulse shape.
	 * b7-b4: Responder transmit pulse shape.
	 * Possible values are: 0x00, 0x01, 0x02, 0x10, 0x11, 0x12, 0x20, 0x21, 0x22.
	 */
	struct {
		/**
		 * @pulse_shape_combos.len: Length of array, number of elements.
		 */
		size_t len;
		/**
		 * @pulse_shape_combos.items: Pointer to items of array.
		 */
		uint8_t *items;
	} pulse_shape_combos;
	/**
	 * @minimum_ran_multiplier: Minimum RAN multiplier supported.
	 *
	 * T_Block RAN = RAN_Multiplier × 96 ms and Time Range = 96 ms to 24480 ms.
	 */
	uint8_t minimum_ran_multiplier;
};

/**
 * enum cherry_ccc_event_type - Type of event for CCC session.
 */
enum cherry_ccc_event_type {
	/**
	 * @CHERRY_CCC_EVENT_TYPE_SESSION_STATUS: CCC session state has
	 * changed.
	 */
	CHERRY_CCC_EVENT_TYPE_SESSION_STATUS,
	/**
	 * @CHERRY_CCC_EVENT_TYPE_SESSION_ERROR: An error occurred with a
	 * session.
	 */
	CHERRY_CCC_EVENT_TYPE_SESSION_ERROR,
	/**
	 * @CHERRY_CCC_EVENT_TYPE_SESSION_CONTROLLER_REPORT: Report of CCC controller session data.
	 *
	 * This event is produced by session for CCC and Aliro user devices.
	 */
	CHERRY_CCC_EVENT_TYPE_SESSION_CONTROLLER_REPORT,
	/**
	 * @CHERRY_CCC_EVENT_TYPE_SESSION_CONTROLEE_REPORT: Report of CCC controlee session data.
	 *
	 * This event is produced by session for CCC vehicle and Aliro responder device.
	 */
	CHERRY_CCC_EVENT_TYPE_SESSION_CONTROLEE_REPORT,
};

/**
 * enum cherry_ccc_session_state - State of a CCC session
 */
enum cherry_ccc_session_state {
	/**
	 * @CHERRY_CCC_SESSION_STATE_INIT: The session is created but not yet
	 * ready to be started.
	 */
	CHERRY_CCC_SESSION_STATE_INIT,
	/**
	 * @CHERRY_CCC_SESSION_STATE_DEINIT: The session is destroyed, it can
	 * no longer be used.
	 */
	CHERRY_CCC_SESSION_STATE_DEINIT,
	/**
	 * @CHERRY_CCC_SESSION_STATE_ACTIVE: The session is started.
	 */
	CHERRY_CCC_SESSION_STATE_ACTIVE,
	/**
	 * @CHERRY_CCC_SESSION_STATE_IDLE: The session is ready to be started
	 * or stopped. It can be stopped either because the max number of
	 * measurements as been reached, or because the session stop was
	 * requested.
	 */
	CHERRY_CCC_SESSION_STATE_IDLE,
};

/**
 * enum cherry_ccc_state_change_reason - Reason why the session state has
 * changed
 */
enum cherry_ccc_state_change_reason {
	/**
	 * @CHERRY_CCC_STATE_CHANGE_REASON_MGMT_CMD: Due to a session
	 * management command
	 */
	CHERRY_CCC_STATE_CHANGE_REASON_MGMT_CMD,
	/**
	 * @CHERRY_CCC_STATE_CHANGE_REASON_UNKNOWN: The reason is unknown
	 */
	CHERRY_CCC_STATE_CHANGE_REASON_UNKNOWN,
	/**
	 * @CHERRY_CCC_STATE_CHANGE_REASON_FORCE_STOPPED: The session
	 * has been force stopped
	 */
	CHERRY_CCC_STATE_CHANGE_REASON_FORCE_STOPPED,
};

/**
 * struct cherry_ccc_session_event_session_status - Data format for the
 * CHERRY_CCC_EVENT_TYPE_SESSION_STATUS event.
 */
struct cherry_ccc_session_event_session_status {
	/**
	 * @session_state: New state of the session.
	 */
	enum cherry_ccc_session_state session_state;
	/**
	 * @reason_code: Reason for the session state change
	 */
	enum cherry_ccc_state_change_reason reason_code;
};

/**
 * struct cherry_ccc_session_event_error - Data format for the
 * CHERRY_CCC_EVENT_TYPE_SESSION_ERROR event.
 */
struct cherry_ccc_session_event_error {
	/**
	 * @status_err: Error code, see the definition of cherry_err.
	 */
	enum cherry_err status_err;
};

/**
* struct cherry_ccc_session_controller_measurements - Ranging measurements for CCC controller.
*/
struct cherry_ccc_session_controller_measurements {
	/**
	 * @next: Next element in list or NULL if last one.
	 */
	struct cherry_ccc_session_controller_measurements *next;
	/**
	 * @frame_status: Zero if OK, or error reason.
	 *
	 * See enum cherry_common_frame_status for all error codes.
	 */
	enum cherry_common_frame_status frame_status;
	/**
	 * @slot_index: Slot index of the measurement.
	 */
	uint8_t slot_index;
	/**
	 * @rr_index: Next ranging round index.
	 */
	uint16_t rr_index;
	/**
	 * @sts_index: STS index of the final frame.
	 */
	uint32_t sts_index;
	/**
	 * @ranging_round: Indicate the first or second active ranging round.
	 *
	 * 0: first ranging round
	 * 1: second ranging round
	 */
	uint8_t ranging_round;
};

/**
* struct cherry_ccc_session_controlee_measurements - Ranging measurements for CCC controlee.
*/
struct cherry_ccc_session_controlee_measurements {
	/**
	 * @next: Next element in list or NULL if last one.
	 */
	struct cherry_ccc_session_controlee_measurements *next;
	/**
	 * @frame_status: Zero if OK, or error reason.
	 *
	 * See enum cherry_common_frame_status for all error codes.
	 */
	enum cherry_common_frame_status frame_status;
	/**
	 * @slot_index: In case of error, slot index where the error was detected.
	 */
	uint8_t slot_index;
	/**
	 * @rr_index: Next Ranging Round index.
	 */
	uint16_t rr_index;
	/**
	 * @sts_index: STS index received in final data message.
	 */
	uint32_t sts_index;
	/**
	 * @distance_cm: Distance in centimeters.
	 *
	 * Speed of light in air value to be used in distance calculation is
	 * 299,702,547 m/s.
	 * If UWBS is unable to calculate distance, then this field is set to 0xFFFF.
	 */
	uint16_t distance_cm;
	/**
	 * @uncertainty_anchor: Ranging timestamp uncertainty of controlee (FOM).
	 */
	uint8_t uncertainty_anchor;
	/**
	 * @uncertainty_initiator: Ranging timestamp uncertainty of controller (FOM).
	 */
	uint8_t uncertainty_initiator;
	/**
	 * @ranging_round: Indicate the first or second active ranging round.
	 *
	 * 0: first ranging round
	 * 1: second ranging round
	 */
	uint8_t ranging_round;
};

/**
 * struct cherry_ccc_controller_session_report - Data format for the
 * CHERRY_CCC_EVENT_TYPE_SESSION_CONTROLLER_REPORT event.
 */
struct cherry_ccc_controller_session_report {
	/**
	 * @sequence_number: session notification counter.
	 */
	uint32_t sequence_number;
	/**
	 * @n_measurements: number of measurements stored in the measurements
	 * table.
	 */
	int n_measurements;
	/**
	 * @measurements: List of ranging measurements.
	 */
	struct cherry_ccc_session_controller_measurements *measurements;
};

/**
 * struct cherry_ccc_controlee_session_report - Data format for the
 * CHERRY_CCC_EVENT_TYPE_SESSION_CONTROLEE_REPORT event.
 */
struct cherry_ccc_controlee_session_report {
	/**
	 * @sequence_number: session notification counter.
	 */
	uint32_t sequence_number;
	/**
	 * @n_measurements: number of measurements stored in the measurements
	 * table.
	 */
	int n_measurements;
	/**
	 * @measurements: List of ranging measurements.
	 */
	struct cherry_ccc_session_controlee_measurements *measurements;
};

/**
 * struct cherry_ccc_event - Cherry CCC event message.
 */
struct cherry_ccc_event {
	/**
	 * @type: Event that occurred.
	 */
	enum cherry_ccc_event_type type;
	/**
	 * @session: Session context.
	 */
	struct cherry_ccc_session *session;
	/**
	 * @data: Pointer to the event data. Each event as it own data format
	 * and size.
	 */
	union {
		/**
		 * @data.status: data for the
		 * CHERRY_CCC_EVENT_TYPE_SESSION_STATUS event.
		 */
		struct cherry_ccc_session_event_session_status *status;
		/**
		 * @data.error: data for the
		 * CHERRY_CCC_EVENT_TYPE_SESSION_ERROR event.
		 */
		struct cherry_ccc_session_event_error *error;
		/**
		 * @data.controller_report: data for the
		 * CHERRY_CCC_EVENT_TYPE_SESSION_CONTROLLER_REPORT event.
		 *
		 * Ranging report produced by session for CCC and Aliro user devices.
		 */
		struct cherry_ccc_controller_session_report *controller_report;
		/**
		 * @data.controlee_report: data for the
		 * CHERRY_CCC_EVENT_TYPE_SESSION_CONTROLEE_REPORT event.
		 *
		 * This event is produced by session for CCC vehicle and Aliro responder device.
		 */
		struct cherry_ccc_controlee_session_report *controlee_report;
	} data;
};

/**
 * typedef cherry_ccc_cb_t - Type for Cherry CCC notification callback.
 * @event: The event type and data.
 * @user_data: Pointer to client data defined at the session creation.
 *
 * This function is part of an experimental feature set and may change or be
 * removed in future releases.
 *
 * The event is allocated by cherry and has to be freed by the application using
 * cherry_ccc_event_free().
 *
 * The callback implementation should return immediately and should not be
 * blocking.
 */
typedef void (*cherry_ccc_cb_t)(struct cherry_ccc_event *event,
				void *user_data);

/**
* enum cherry_ccc_hopping_mode - Hopping mode for CCC session.
*
* @CHERRY_CCC_HOPPING_MODE_DISABLE: No hopping.
* @CHERRY_CCC_HOPPING_MODE_ENABLED: Hopping enabled.
* @CHERRY_CCC_HOPPING_MODE_CONTINUOUS_AES: Continuous hopping with AES-based hopping sequence.
* @CHERRY_CCC_HOPPING_MODE_CONTINUOUS_DEFAULT: Continuous hopping with default hopping sequence.
* @CHERRY_CCC_HOPPING_MODE_ADAPTATIVE_AES: Adaptative hopping with AES-based hopping sequence.
* @CHERRY_CCC_HOPPING_MODE_ADAPTATIVE_DEFAULT: Adaptative hopping with default hopping sequence.
*/
enum cherry_ccc_hopping_mode {
	CHERRY_CCC_HOPPING_MODE_DISABLE = 0,
	CHERRY_CCC_HOPPING_MODE_ENABLED = 1,
	CHERRY_CCC_HOPPING_MODE_CONTINUOUS_AES = 0xA0,
	CHERRY_CCC_HOPPING_MODE_CONTINUOUS_DEFAULT = 0xA1,
	CHERRY_CCC_HOPPING_MODE_ADAPTATIVE_AES = 0xA2,
	CHERRY_CCC_HOPPING_MODE_ADAPTATIVE_DEFAULT = 0xA3,
};

/**
* struct cherry_ccc_aliro_session_config - Basic configuration parameters for Aliro
* sessions.
*
* Those parameters are negotiated over BLE between the User device and Reader according to each
* other's capabilities.
*/
struct cherry_ccc_aliro_session_config {
	/**
	 * @session_id: Session identifier, any non-zero value that must match with
	 * peers.
	 */
	uint32_t session_id;
	/**
	 * @uwb_config_id: UWB configuration identifier.
	 */
	uint16_t uwb_config_id;
	/**
	 * @pulse_shape_combo: Pulse shape combinations.
	 */
	uint8_t pulse_shape_combo;
	/**
	 * @channel: Channel number.
	 */
	uint8_t channel;
	/**
	 * @sync_code_index: SYNC code index.
	 *
	 * Same as preamble code index.
	 */
	uint8_t sync_code_index;
	/**
	 * @ranging_duration_ms: Interval between two ranging in milliseconds.
	 */
	uint32_t ranging_duration_ms;
	/**
	 * @slot_per_rr: Number of slots per ranging round.
	 */
	uint8_t slot_per_rr;
	/**
	 * @slot_duration: Duration of a ranging slot in the unit of RSTU.
	 *
	 * 1200 RSTU is equal to 1ms.
	 */
	uint16_t slot_duration;
	/**
	 * @hopping_mode: Ranging round hopping configuration
	 */
	enum cherry_ccc_hopping_mode hopping_mode;
	/**
	 * @hopping_config_bitmask: Hopping capabilities config bitmask
	 */
	uint8_t hopping_config_bitmask;
	/**
	 * @hop_mode_key: Key to generate hopping sequence.
	 */
	uint8_t hop_mode_key[CHERRY_CCC_HOP_MODE_KEY_SIZE];
	/**
	 * @sts_index: The starting STS index.
	 */
	uint32_t sts_index;
	/**
	 * @mac_mode: Specify if one or multiple ranging round are used in a ranging block.
	 *
	 *  b0-b5: Offset between the 2 ranging ranging blocks.
	 *  b5-b6: Number of ranging round used: 0b00 -> 1 ranging round, 0b01 -> 2 ranging round.
	 */
	uint8_t mac_mode;
	/**
	 * @uwb_time_us: Starting time reference on UWBs time base of ranging session.
	 *
	 * In microseconds, 0 to start immediately.
	 */
	uint64_t uwb_time_us;
};

/**
 * cherry_ccc_session_create_aliro_initiator() - Create and configure a CCC
 * session for an Aliro Initiator device.
 * @ctx: Cherry context.
 * @callback: Callback function to be call for session events.
 * @user_data: Optional pointer to data element managed by the Cherry's client.
 *             It is the pointer that will be put back as argument of CCC event
 *             callback.
 * @config: Basic session configuration parameters.
 * @n_responder: Number of responders.
 *
 * This function is part of an experimental feature set and may change or be
 * removed in future releases.
 *
 * This function allocates the session context which must be released using
 * cherry_ccc_session_destroy(). Then it setups every needed parameters to
 * prepare the CCC session.
 *
 * This function returns immediately.
 *
 * Returns: Newly allocated session or NULL on error.
 */
struct cherry_ccc_session *cherry_ccc_session_create_aliro_initiator(
	struct cherry *ctx, cherry_ccc_cb_t callback, void *user_data,
	struct cherry_ccc_aliro_session_config *config, uint8_t n_responder);

/**
 * cherry_ccc_session_create_aliro_responder() - Create and configure a CCC
 * session for an Aliro Responder device.
 * @ctx: Cherry context.
 * @callback: Callback function to be call for session events.
 * @user_data: Optional pointer to data element managed by the Cherry's client.
 *             It is the pointer that will be put back as argument of CCC event
 *             callback.
 * @config: Basic session configuration parameters.
 *
 * This function is part of an experimental feature set and may change or be
 * removed in future releases.
 *
 * This function allocates the session context which must be released using
 * cherry_ccc_session_destroy(). Then it setups every needed parameters to
 * prepare the CCC session.
 *
 * This function returns immediately.
 *
 * Returns: Newly allocated session or NULL on error.
 */
struct cherry_ccc_session *cherry_ccc_session_create_aliro_responder(
	struct cherry *ctx, cherry_ccc_cb_t callback, void *user_data,
	struct cherry_ccc_aliro_session_config *config);

/**
 * cherry_ccc_session_to_base() - Return the base session object
 * associated to a CCC one.
 * @session: CCC session object.
 *
 * This function is part of an experimental feature set and may change or be
 * removed in future releases.
 *
 * Returns: Generic session object.
 */
struct cherry_session *
cherry_ccc_session_to_base(struct cherry_ccc_session *session);

/**
 * cherry_ccc_session_get_user_data() - Get the user_data of the session
 * @session: Session object.
 *
 * This function is part of an experimental feature set and may change or be
 * removed in future releases.
 *
 * Returns: The user_data pointer given at the session creation.
 */
static inline void *
cherry_ccc_session_get_user_data(struct cherry_ccc_session *session)
{
	return cherry_session_get_user_data(
		cherry_ccc_session_to_base(session));
}

/**
 * cherry_ccc_event_free() - Release the event allocated memory.
 * @event: Pointer to the event to be freed.
 *
 * This function is part of an experimental feature set and may change or be
 * removed in future releases.
 *
 * After each call to @cherry_ccc_cb_t, this function has to be used to free
 * the corresponding event data. It lets the upper layer decide either to fully
 * handle the event inside the callback or to defer some processing outside
 * without temporary data copy.
 */
void cherry_ccc_event_free(struct cherry_ccc_event *event);

/**
 * cherry_ccc_session_destroy() - Release a CCC session.
 * @session: Session object.
 *
 * This function is part of an experimental feature set and may change or be
 * removed in future releases.
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
static inline void
cherry_ccc_session_destroy(struct cherry_ccc_session *session)
{
	cherry_session_destroy(cherry_ccc_session_to_base(session));
}

/**
 * cherry_ccc_session_start() - Start a CCC session.
 * @session: Session object.
 *
 * This function is part of an experimental feature set and may change or be
 * removed in future releases.
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
 */
static inline enum cherry_err
cherry_ccc_session_start(struct cherry_ccc_session *session)
{
	return cherry_session_start(cherry_ccc_session_to_base(session));
}

/**
 * cherry_ccc_session_stop() - Stop a CCC session.
 * @session: Session object.
 *
 * This function is part of an experimental feature set and may change or be
 * removed in future releases.
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
cherry_ccc_session_stop(struct cherry_ccc_session *session)
{
	return cherry_session_stop(cherry_ccc_session_to_base(session));
}

/**
 * cherry_ccc_session_set_antennas() - Select the Antenna Set to be used for the session
 * @session: Session object.
 * @tx_antenna_set: Antenna set to be used as Tx for the 1st ranging round.
 * @rx_antenna_set: Antenna set to be used as Rx for the 1st ranging round.
 *
 * This function is part of an experimental feature set and may change or be
 * removed in future releases.
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
static inline enum cherry_err
cherry_ccc_session_set_antennas(struct cherry_ccc_session *session,
				uint8_t tx_antenna_set, uint8_t rx_antenna_set)
{
	return cherry_session_set_antennas(cherry_ccc_session_to_base(session),
					   tx_antenna_set, rx_antenna_set);
}

/**
 * DOC: Extra APIs for CCC fine session configuration
 */

/**
 * cherry_ccc_session_set_ursk() - Set the UWB Ranging Secret Key
 * @session: Session object.
 * @ursk: Pointer a 32 bytes STS root key.
 *
 * This function is part of an experimental feature set and may change or be
 * removed in future releases.
 *
 * This function may be called at any time, as long as the session is not
 * active. It is only applicable to Aliro sessions when the URSK is provided
 * by the UWBS Host and not by a Secure Element.
 *
 * This function returns immediately.
 *
 * Returns:
 *  - &CHERRY_ERR_NONE if accepted
 *  - &CHERRY_ERR_INVALID_PARAMETER on any invalid parameter
 */
enum cherry_err cherry_ccc_session_set_ursk(struct cherry_ccc_session *session,
					    const uint8_t *ursk);

/**
 * cherry_ccc_session_set_round2_antennas() - Set the antennas to be used for
 * Aliro's 2nd round.
 * @session: Session object.
 * @tx_antenna_set: Antenna set to be used as Tx for the 2nd ranging round.
 * @rx_antenna_set: Antenna set to be used as Rx for the 2nd ranging round.
 *
 * This function is part of an experimental feature set and may change or be
 * removed in future releases.
 *
 * This function may be called at any time, as long as the session is not
 * active. But this function is only applicable for Aliro responder sessions.
 * It does not have to be called if no change is needed from default
 * parameters.
 *
 * When the Aliro MAC mode 1 is selected, 2 ranging rounds are used during a
 * single
 * ranging block. This function allows to select the antenna to be used for the
 * second ranging round.
 *
 * This function returns immediately.
 *
 * Returns:
 *  - &CHERRY_ERR_NONE if accepted
 *  - &CHERRY_ERR_INVALID_PARAMETER on any invalid parameter
 */
enum cherry_err
cherry_ccc_session_set_round2_antennas(struct cherry_ccc_session *session,
				       uint8_t tx_antenna_set,
				       uint8_t rx_antenna_set);

/**
 * cherry_ccc_session_set_sts_index() - Set the STS index.
 *
 * @session: Session object.
 * @sts_index: STS index.
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
enum cherry_err
cherry_ccc_session_set_sts_index(struct cherry_ccc_session *session,
				 uint32_t sts_index);

/**
 * cherry_ccc_session_set_initiation_time() - Set the initiation time.
 *
 * @session: Session object.
 * @initiation_time_us: Starting time reference on UWBS time base of ranging session.
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
enum cherry_err
cherry_ccc_session_set_initiation_time(struct cherry_ccc_session *session,
				       uint64_t initiation_time_us);

#ifdef __cplusplus
}
#endif
