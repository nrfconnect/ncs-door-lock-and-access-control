/**
 * DOC: Interface for the Aliro UWB session handled by the Aliro UWB adapter.
 *
 * SPDX-FileCopyrightText: Copyright (c) 2025 Qorvo, Inc.
 * SPDX-License-Identifier: LicenseRef-Qorvo-SDK-1
 *
 */

#pragma once

#include <cherry/cherry.h>
#include <cherry/cherry_ccc.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * DOC: struct aliro_uwb_session
 *
 * Object representing an Aliro UWB session.
 *
 * All sessions are associated with a session object, which is given to every
 * functions working with a session. It contains the session context. It is
 * created by the aliro_uwb_session_create() function and destroyed by the
 * aliro_uwb_session_destroy() function.
 */
struct aliro_uwb_session;

/**
 * struct aliro_uwb_message - Aliro UWB message.
 */
struct aliro_uwb_message {
	/**
	 * @len: Length of the message.
	 *
	 * Number of bytes into &data.
	 */
	size_t len;
	/**
	 * @data: Pointer to the message data.
	 */
	uint8_t data[];
};

/**
 * enum aliro_uwb_session_event_type - Type of event for Aliro session.
 */
enum aliro_uwb_session_event_type {
	/**
	 * @ALIRO_UWB_SESSION_EVENT_TYPE_SESSION_STATUS: Aliro session state has changed.
	 */
	ALIRO_UWB_SESSION_EVENT_TYPE_SESSION_STATUS = CHERRY_CCC_EVENT_TYPE_SESSION_STATUS,
	/**
	 * @ALIRO_UWB_SESSION_EVENT_TYPE_SESSION_ERROR: An error occurred with a session.
	 */
	ALIRO_UWB_SESSION_EVENT_TYPE_SESSION_ERROR = CHERRY_CCC_EVENT_TYPE_SESSION_ERROR,
	/**
	 * @ALIRO_UWB_SESSION_EVENT_TYPE_SESSION_CONTROLLER_REPORT: Report of Aliro controller
	 * session data.
	 *
	 * This event is produced by session for Aliro user devices.
	 */
	ALIRO_UWB_SESSION_EVENT_TYPE_SESSION_CONTROLLER_REPORT =
		CHERRY_CCC_EVENT_TYPE_SESSION_CONTROLLER_REPORT,
	/**
	 * @ALIRO_UWB_SESSION_EVENT_TYPE_SESSION_CONTROLEE_REPORT: Report of Aliro controlee session
	 * data.
	 *
	 * This event is produced by session for Aliro responder device.
	 */
	ALIRO_UWB_SESSION_EVENT_TYPE_SESSION_CONTROLEE_REPORT =
		CHERRY_CCC_EVENT_TYPE_SESSION_CONTROLEE_REPORT,
	/**
	 * @ALIRO_UWB_SESSION_EVENT_TYPE_SESSION_DIAGNOSTIC_REPORT: Report of Aliro UWB diagnotics
	 * data.
	 *
	 * This event is produced by session for Aliro device as soon as the diagnotics have been
	 * enabled.
	 */
	ALIRO_UWB_SESSION_EVENT_TYPE_SESSION_DIAGNOSTIC_REPORT =
		CHERRY_CCC_EVENT_TYPE_SESSION_DIAGNOSTIC_REPORT,
};

/**
 * struct aliro_uwb_session_event - Aliro UWB session event.
 */
struct aliro_uwb_session_event {
	/**
	 * @session: Session context.
	 */
	struct aliro_uwb_session *session;
	/**
	 * @type: Event that occurred.
	 */
	enum aliro_uwb_session_event_type type;
	/**
	 * @cherry_event: Pointer to the parent Cherry event.
	 */
	void *cherry_event;
	/**
	 * @data: Pointer to the event data.
	 *
	 * Each event as it own data format and size indicated by @type.
	 */
	union {
		/**
		 * @data.status: data for the
		 * ALIRO_UWB_SESSION_EVENT_TYPE_SESSION_STATUS event.
		 */
		struct cherry_ccc_session_event_session_status *status;
		/**
		 * @data.error: data for the
		 * ALIRO_UWB_SESSION_EVENT_TYPE_SESSION_ERROR event.
		 */
		struct cherry_ccc_session_event_error *error;
		/**
		 * @data.controller_report: data for the
		 * ALIRO_UWB_SESSION_EVENT_TYPE_SESSION_CONTROLLER_REPORT event.
		 *
		 * Ranging report produced by session for Aliro user devices.
		 */
		struct cherry_ccc_controller_session_report *controller_report;
		/**
		 * @data.controlee_report: data for the
		 * ALIRO_UWB_SESSION_EVENT_TYPE_SESSION_CONTROLEE_REPORT event.
		 *
		 * This event is produced by session for Aliro reader device.
		 */
		struct cherry_ccc_controlee_session_report *controlee_report;
		/**
		 * @data.diagnostics: data for the
		 * ALIRO_UWB_SESSION_EVENT_TYPE_SESSION_DIAGNOSTIC_REPORT event.
		 */
		struct cherry_common_diag_report *diagnostics;
	} data;
};

/**
 * typedef aliro_uwb_session_cb_t - Type for Aliro UWB session notification callback.
 * @event: The event type and data.
 * @user_data: Pointer to client data defined at the session creation.
 *
 * The event is allocated by Aliro UWB adapter and has to be freed by the application using
 * aliro_uwb_session_event_free().
 *
 * The callback implementation should return immediately and should not be
 * blocking.
 */
typedef void (*aliro_uwb_session_cb_t)(struct aliro_uwb_session_event *event, void *user_data);

/**
 * typedef aliro_uwb_adapter_transmit_message_t - Type for Aliro message transmission over BLE.
 * @message: The event type and data.
 * @session: Session object.
 * @user_data: Pointer to client data defined at the session creation.
 * @timeout: Indicates if the message has a response timeout.
 *
 * The event is allocated by Aliro UWB adapter and has to be freed by the application using
 * aliro_uwb_session_message_free().
 *
 * The callback implementation should return immediately and should not be
 * blocking.
 */
typedef void (*aliro_uwb_adapter_transmit_message_t)(struct aliro_uwb_message *message,
						     struct aliro_uwb_session *session,
						     void *user_data, bool timeout);

/**
 * aliro_uwb_session_create() - Create Aliro UWB session.
 * @aliro_ctx: Aliro UWB adapter context.
 * @session_id: Session identifier, which is the least significant four octets
 *             of the Transaction Identifier field.
 * @callback: Callback function to be call for session events.
 * @transmit: function call to transmit Aliro message over BLE.
 * @user_data: Optional pointer to data element managed by the Aliro adapter's client.
 *             It is the pointer that will be put back as argument of the session event callback.
 *
 * This function allocates the session context which must be released using
 * aliro_uwb_session_destroy(). Then it setups every needed parameters to
 * prepare the Aliro session.
 *
 * This function returns immediately.
 *
 * Returns: Newly allocated session or NULL on error.
 */
struct aliro_uwb_session *aliro_uwb_session_create(struct aliro_uwb_adapter *aliro_ctx,
						   uint32_t session_id,
						   aliro_uwb_session_cb_t callback,
						   aliro_uwb_adapter_transmit_message_t transmit,
						   void *user_data);

/**
 * aliro_uwb_session_destroy() - Release an Aliro UWB session.
 * @session: Session object.
 *
 * This can be used on any Aliro session, active or not. The session will be
 * stopped if needed and removed from the UWB subsystem. The session is not
 * destroyed immediately, it is marked as destructed and can no longer be used
 * by the API client. The session context will be freed after being removed
 * from the UWB subsystem.
 *
 * The session object must not be used after this call.
 *
 * This function returns immediately.
 */
void aliro_uwb_session_destroy(struct aliro_uwb_session *session);

/**
 * aliro_uwb_session_message_free() - Release the message allocated memory.
 * @message: Pointer to the message to be freed.
 *
 * After each call to @aliro_uwb_adapter_transmit_message_t, this function has to be used to free
 * the corresponding message data. It lets the upper layer decide either to fully
 * handle the message inside the callback or to defer some processing outside
 * without temporary data copy.
 */
void aliro_uwb_session_message_free(struct aliro_uwb_message *message);

/**
 * aliro_uwb_session_event_free() - Release the event allocated memory.
 * @event: Pointer to the event to be freed.
 *
 * After each call to @aliro_uwb_session_cb_t, this function has to be used to free
 * the corresponding event data. It lets the upper layer decide either to fully
 * handle the event inside the callback or to defer some processing outside
 * without temporary data copy.
 */
void aliro_uwb_session_event_free(struct aliro_uwb_session_event *event);

/**
 * aliro_uwb_session_set_ursk() - Set the UWB Ranging Secret Key
 * @session: Session object.
 * @ursk: Pointer a 32 bytes STS root key.
 *
 * This function returns immediately.
 *
 * Returns:
 *  - &ALIRO_UWB_ERR_NONE if succeed
 *  - &ALIRO_UWB_ERR_INTERNAL for other errors
 */
enum aliro_uwb_err aliro_uwb_session_set_ursk(struct aliro_uwb_session *session,
					      const uint8_t *ursk);

/**
 * aliro_uwb_session_set_protocol_version() - Set the UWB Ranging protocol version
 * @session: Session object.
 * @selected_protocol_version: uwb selected protocol version
 *
 * This function returns immediately.
 *
 * Returns:
 *  - &ALIRO_UWB_ERR_NONE if succeed
 *  - &ALIRO_UWB_ERR_INTERNAL for other errors
 */
enum aliro_uwb_err aliro_uwb_session_set_protocol_version(struct aliro_uwb_session *session,
							  uint16_t selected_protocol_version);

/**
 * aliro_uwb_session_init_setup() - Start the set up of the Aliro UWB session.
 * @session: Session object.
 *
 * The session must be created before starting its setup. This create the first Aliro UWB setup
 * message called M1 and returns it to the caller. This M1 message must be sent over BLE to the user
 * device.
 *
 * Returns:
 *  - &ALIRO_UWB_ERR_NONE if succeed
 *  - &ALIRO_UWB_ERR_INTERNAL for other errors
 */
enum aliro_uwb_err aliro_uwb_session_init_setup(struct aliro_uwb_session *session);

/**
 * aliro_uwb_session_set_time_offset() - Set the time offset with the User Device.
 * @session: Session object.
 * @time_offset: Time offset, in microseconds, to be added to UWB_Time0.
 *
 * Returns:
 *  - &ALIRO_UWB_ERR_NONE if succeed
 *  - &ALIRO_UWB_ERR_INTERNAL for other errors
 */
enum aliro_uwb_err aliro_uwb_session_set_time_offset(struct aliro_uwb_session *session,
						     int64_t time_offset);

/**
 * aliro_uwb_session_message_handle() - Handle an incoming message.
 * @session: session object.
 * @message: message to be handled.
 *
 * Process the incoming message and take the appropriate action. This function
 * may return a message to be sent back to the user device. The caller is responsible to send the
 * response message over BLE.
 *
 * In case of error, the session callback is called with the error event including an error code.
 *
 * Returns:
 *  - &ALIRO_UWB_ERR_NONE if succeed
 *  - &ALIRO_UWB_ERR_INTERNAL for other errors
 */
enum aliro_uwb_err aliro_uwb_session_message_handle(struct aliro_uwb_session *session,
						    struct aliro_uwb_message *message);

/**
 * aliro_uwb_session_suspend - Requests suspend of the given UWB session.
 * @session: Pointer to the UWB session context to be suspended.
 *
 * This function prepare a suspend message for the specified UWB session. This message must be sent
 * over BLE by the caller to the other Aliro device. Which will reply with a suspend response
 * message containing a status. According to the status, the session will be suspend or not by the
 * aliro_uwb_session_message_handle() which will handle it.
 *
 * Returns:
 *  - &ALIRO_UWB_ERR_NONE if succeed
 *  - &ALIRO_UWB_ERR_INTERNAL for other errors
 */
enum aliro_uwb_err aliro_uwb_session_suspend(struct aliro_uwb_session *session);

/**
 * aliro_uwb_session_forced_suspend - Suspends immediately the given UWB session.
 * @session: Pointer to the UWB session context to be suspended.
 *
 * This function suspend the UWB ranging and notify the peer device the session has been suspened.
 *
 * Returns:
 *  - &ALIRO_UWB_ERR_NONE if succeed
 *  - &ALIRO_UWB_ERR_INTERNAL for other errors
 */
enum aliro_uwb_err aliro_uwb_session_forced_suspend(struct aliro_uwb_session *session);

/**
 * aliro_uwb_session_resume - Resumes the given UWB session.
 * @session: Pointer to the UWB session context to be resumed.
 *
 * This function prepare a resume message for the specified UWB session. This message must
 * be sent over BLE by the caller to the other Aliro device. Which will reply with a resume
 * response message containing a status. According to the status, the session will be
 * suspend or not by the aliro_uwb_session_message_handle() which will handle it.
 *
 * Returns:
 *  - &ALIRO_UWB_ERR_NONE if succeed
 *  - &ALIRO_UWB_ERR_INTERNAL for other errors
 */
enum aliro_uwb_err aliro_uwb_session_resume(struct aliro_uwb_session *session);

#ifdef __cplusplus
}
#endif
