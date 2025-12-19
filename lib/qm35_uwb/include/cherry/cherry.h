/*
 * SPDX-FileCopyrightText: Copyright (c) 2025 Qorvo, Inc.
 * SPDX-License-Identifier: LicenseRef-Qorvo-SDK-1
 */

#pragma once

#include "cherry_common.h"

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * DOC:
 *
 * Cherry is an opinionated facade to UWB subsystem. It takes sane decisions
 * about parameters to use for ranging in order to provide a turn-key solution
 * to API users.
 */

/**
 * DOC: Version
 * The cherry version is defined by @CHERRY_VERSION:
 *  - major: CHERRY_VERSION / 10000,
 *  - minor: CHERRY_VERSION / 100 % 100,
 *  - patch: CHERRY_VERSION % 100.
 */
#define CHERRY_VERSION 021800

/**
 * DOC: Changelog
 *
 * Version 2.18.00:
 *  - Add parsing of FiRa message ID in unified diagnostics.
 *  - Remove useless directives in cherry.c and cherry_session_manager.c.
 *  - Add robustness:
 *    Add NULL pointer checks in session and core APIs.
 *    Prevent integer overflow in task params allocation.
 *    Add NULL check before callback invocation.
 *    Add NULL checks in session diagnostics and ref time APIS.
 *    Prevent buffer overflow in dt anchor responders array.
 *    Handle internal state only in Cherry thread, avoiding concurrency issues
 *    between threads.
 *    Add error log in case session pointer is null in some APIs.
 *    Use missed mutex in @cherry_session_set_diagnostics.
 *    Prevent buffer overflow in twr controller addresses.
 *    Prevent null dereference in wait functions.
 *    Prevent race condition on cherry_ctx access.
 *    Unlock the mutex before calling @cherry_session_free to ensure proper
 *    mutex lifecycle management.
 *    Prevent null dereference in error state update.
 *    Prevent race condition in force stop all sessions.
 *    Add null checks for critical session callbacks.
 *    Clean up thread on UCI init failure error path.
 *    Prevent unbounded tap data allocation in diagnostic parser.
 *  - Fix null pointer dereference in @cherry_proxy_signal.
 *  - Add timestamp to logging.
 *  - Fix IE discovery V2 parsing in rtls client.
 *  - Fix segfault when notifications remain in queue:
 *    Check if a session is registered before sending an event.
 *    Fix possible memory leak when removing tasks from Cherry thread.
 *    Fix force stop in case of multi/interleaving sessions.
 *    Move cherry_fira_get_child() calls in Cherry tasks.
 *  - Allow logging to go to qm35_daemon through UNIX socket:
 *    The CHERRY_LOG environment variable specifies a regular file or an
 *    UNIX socket where to send log. If unset, it default to the default
 *    path the qm35_daemon use: /run/qm35_daemon/syslog.socket
 *    If set, the given path is checked to detect its type and if it is an
 *    UNIX socket, a connection is started and the cherry_log_output is
 *    created to use the connection file descriptor.
 *    If given path do not exist or is not an UNIX socket, the given path is
 *    used as a regular file.
 *    If given path is empty string, cherry_log_output stays unset and logs
 *    are sent to stderr.
 *  - Allocate RTLS v2 elements independently and parse separately:
 *    Fix memory leaks for allocation/deallocation mismatch.
 *
 * Version 2.17.03:
 *  - Notify an error when STOP or DEINIT failed after a
 *    cherry_xxx_session_destroy() call.
 *    If a timeout occured the sessions are forced stopped to notify the DEINIT
 *    to the applicative.
 *
 * Version 2.17.02:
 *  - Set UCI RSP timeout to 2s.
 *
 * Version 2.17.01:
 *  - Disable API for CIR window size and offset configuration until
 *    supported in FW.
 *  - Fix CIR parsing in diagnostics handler.
 *
 * Version 2.17.00:
 *  - Implement unified diagnostic notification:
 *	  new fields in @cherry_common_diag_frame structure.
 *	  new fields in @cherry_common_segment_metrics structure.
 *  - Add @cherry_session_set_diagnostics API.
 *
 * Version 2.16.00:
 *  - Add API to set dltdoa hop count:
 *    @cherry_fira_session_set_dltdoa_hop_count.
 *
 * Version 2.15.00:
 *  - Add API for punctured HPRF radar:
 *    @cherry_radar_session_set_preamble_duration.
 *    @cherry_radar_session_set_rframe_config.
 *    @cherry_radar_session_set_preamble_code_index.
 *
 * Version 2.14.00:
 *  - Add @cherry_ccc_session_set_sts_index API.
 *  - Add @cherry_ccc_session_set_initiation_time API.
 *
 * Version 2.13.00:
 *  - Add DL-TDOA notfication v2 support.
 *
 * Version 2.12.00:
 *  - Add Tx RCP init notification setter.
 *  - Add Tx RCP init notification management for RTLS.
 *
 * Version 2.11.00:
 *  - Add GTSW allocations features:
 * 	  @cherry_rtls_session_update_gtsw_rx_allocations API.
 *
 * Version 2.10.00:
 *  - Add time sync feature:
 * 	  @cherry_toggle_gpio_time_sync API.
 *
 * Version 2.09.00:
 *  - Add experimental support of extended DL-TDoA anchor coordinate formats.
 *  - Update of CCC and Aliro experimental APIs.
 *
 * Version 2.08.00:
 *  - Add CCC and Aliro experimental APIs.
 *  - Add NLOS and RSSI in RTLS tag measurements.
 *  - Report NLOS in TWR measurements.
 *  - Add CHERRY_ERR_SESSION_TYPE_NOT_SUPPORTED.
 *  - Fix dl-tdoa coordonates.
 *  - Add API for advanced RTLS:
 *    @cherry_rtls_session_set_parents API.
 *    @cherry_rtls_session_set_discovery_tx API.
 *    @cherry_rtls_session_set_neighbors_tofs API.
 *  - Add @cherry_get_device_timestamp API.
 *  - Add @cherry_get_device_capabilities API.
 *  - Add CCC APIs.
 *
 * Version 2.07.00:
 *  - Add the @cherry_get_calib API.
 *
 * Version 2.06.00:
 *  - Remove deprecated cherry_destroy API.
 *
 * Version 2.05.00:
 *  - Add the @cherry_rtls_session_set_rcp_init_rx_slots API.
 *
 * Version 2.04.00:
 *  - Add the @cherry_fira_session_set_dltdoa_tx_active_rr API.
 *
 * Version 2.03.00:
 *  - Add Radar APIs.
 *
 * Version 2.02.00:
 *  - Add the @cherry_country_code_open/close APIs.
 *  - Fix typo on CHERRY_COMMON_PRF_MODE_HPRF_249_6M
 *  - Create generic session APIs
 *  - Create RTLS APIs
 *  - Add API to configure session's reference time base
 *
 * Version 2.01.00:
 *  - Add the @cherry_fira_session_set_slots_per_ranging_round API.
 *
 * Version 2.00.00:
 *  - WARNING: The cherry session context ownership has been modified. Cherry doesn't release
 *    anymore the session context in case of error or reset. The @cherry_fira_session_destroy
 *    should always be called even after @cherry_reset_device.
 *  - WARNING: The cherry_destroy is now deprecated, it will be removed in next major release.
 *    @cherry_destroy_sync should be used instead.
 *  - Add DL-TDoA APIs for anchor and tag session creation.
 *  - Add the cherry proxy APIs.
 *  - Add the @cherry_fira_session_set_antenna API.
 *  - Add the @cherry_set_log_level API.
 *  - Add the @cherry_fira_session_set_phy API.
 */

/**
 * DOC: struct cherry
 *
 * Cherry context object.
 *
 * It hold the Cherry runtime context, created at the
 * initialization and released at the close of Cherry.
 */
struct cherry;

/**
 * enum cherry_err - Status code returned by Cherry.
 */
enum cherry_err {
	/**
	 * @CHERRY_ERR_NONE: No error.
	 */
	CHERRY_ERR_NONE,
	/**
	 * @CHERRY_ERR_INVALID_PARAMETER: Invalid parameter.
	 */
	CHERRY_ERR_INVALID_PARAMETER,
	/**
	 * @CHERRY_ERR_UWBS_TIMEOUT: Didn't get the UWBS respond to an UCI command
	 * before the time limit.
	 */
	CHERRY_ERR_UWBS_TIMEOUT,
	/**
	 * @CHERRY_ERR_INTERNAL: An internal error occurred.
	 */
	CHERRY_ERR_INTERNAL,
	/**
	 * @CHERRY_ERR_SESSION_INIT: Initialization of the new session has
	 * failed.
	 */
	CHERRY_ERR_SESSION_INIT,
	/**
	 * @CHERRY_ERR_SESSION_ACTIVE: Operation can not be done on active
	 * session.
	 */
	CHERRY_ERR_SESSION_ACTIVE,
	/**
	 * @CHERRY_ERR_SESSION_CONFIG: The session can not be started because
	 * it's missing some configurations.
	 */
	CHERRY_ERR_SESSION_CONFIG,
	/**
	 * @CHERRY_ERR_SESSION_TYPE_NOT_SUPPORTED: The session type is not supported
	 * by UWBS.
	 */
	CHERRY_ERR_SESSION_TYPE_NOT_SUPPORTED,
};

/**
 * enum cherry_core_event_type - Cherry event type not related to a session.
 */
enum cherry_core_event_type {
	/**
	 * @CHERRY_CORE_EVENT_TYPE_DEVICE_STATUS: Update of the UWBS device
	 * state.
	 */
	CHERRY_CORE_EVENT_TYPE_DEVICE_STATUS,
	/**
	 * @CHERRY_CORE_EVENT_TYPE_ERROR: An error occurred, the UWBS may needs
	 * a reset.
	 */
	CHERRY_CORE_EVENT_TYPE_ERROR,
	/**
	 * @CHERRY_CORE_EVENT_TYPE_CALIB_UPDATE: The UWBS calibration update
	 * request ended.
	 */
	CHERRY_CORE_EVENT_TYPE_CALIB_UPDATE,
	/**
	 * @CHERRY_CORE_EVENT_TYPE_DEVICE_INFO: UWBS device information.
	 */
	CHERRY_CORE_EVENT_TYPE_DEVICE_INFO,
	/**
	 * @CHERRY_CORE_EVENT_TYPE_DEVICE_STATS: UWBS device stats.
	 */
	CHERRY_CORE_EVENT_TYPE_DEVICE_STATS,
	/**
	 * @CHERRY_CORE_EVENT_TYPE_GET_CALIB: Cherry retrieved requested calibration keys.
	 */
	CHERRY_CORE_EVENT_TYPE_GET_CALIB,
	/**
	 * @CHERRY_CORE_EVENT_TYPE_DEVICE_CAPS: UWBS device capabilities
	 */
	CHERRY_CORE_EVENT_TYPE_DEVICE_CAPS,
	/**
	 * @CHERRY_CORE_EVENT_TYPE_TIMESTAMP: UWBS timestamp
	 */
	CHERRY_CORE_EVENT_TYPE_TIMESTAMP,
	/**
	 * @CHERRY_CORE_EVENT_TYPE_GPIO_TOGGLE: Gpio toggle information.
	 */
	CHERRY_CORE_EVENT_TYPE_GPIO_TOGGLE,
};

/**
 * enum cherry_core_device_state - State of the UWBS provided through the
 * CHERRY_CORE_EVENT_TYPE_DEVICE_STATUS event
 */
enum cherry_core_device_state {
	/**
	 * @CHERRY_CORE_DEVICE_STATE_READY: UWBS is initialized and ready for
	 * UWB session.
	 */
	CHERRY_CORE_DEVICE_STATE_READY = 0x1,
	/**
	 * @CHERRY_CORE_DEVICE_STATE_ACTIVE: UWBS is busy with on going UWB
	 * session.
	 */
	CHERRY_CORE_DEVICE_STATE_ACTIVE,
	/**
	 * @CHERRY_CORE_DEVICE_STATE_ERROR: Error occurred with the UWBS.
	 */
	CHERRY_CORE_DEVICE_STATE_ERROR,
};

/**
 * enum cherry_core_state_change_reason - Boot reason of the UWBS provided through the
 * CHERRY_CORE_EVENT_TYPE_DEVICE_STATUS event
 */
enum cherry_core_state_change_reason {
	/**
	 * @CHERRY_CORE_STATE_CHANGE_ACTIVITY: UWBS changes ready/active.
	 */
	CHERRY_CORE_STATE_CHANGE_ACTIVITY = 0x1,
	/**
	 * @CHERRY_CORE_STATE_CHANGE_BOOT_REASON_UNKNOWN: UWBS boot for unknow reason.
	 */
	CHERRY_CORE_STATE_CHANGE_BOOT_REASON_UNKNOWN = 0x2,
	/**
	 * @CHERRY_CORE_STATE_CHANGE_BOOT_REASON_FATAL: UWBS reset after a fatal error.
	 */
	CHERRY_CORE_STATE_CHANGE_BOOT_REASON_FATAL = 0x3,
	/**
	 * @CHERRY_CORE_STATE_CHANGE_SOFT_RESET: UWBS received soft reset command.
	 */
	CHERRY_CORE_STATE_CHANGE_SOFT_RESET = 0x4
};

/**
 * enum cherry_core_gpio_toggle_mode - Mode to use for gpio toggle.
 */
enum cherry_core_gpio_toggle_mode {
	/**
	 * @CHERRY_CORE_GPIO_TOGGLE_MODE_SINGLE: gpio toggle mode single.
	 */
	CHERRY_CORE_GPIO_TOGGLE_MODE_SINGLE,
	/**
	 * @CHERRY_CORE_GPIO_TOGGLE_MODE_DEFAULT: gpio toggle default mode.
	 */
	CHERRY_CORE_GPIO_TOGGLE_MODE_DEFAULT =
		CHERRY_CORE_GPIO_TOGGLE_MODE_SINGLE,
};

/**
 * struct cherry_core_event_device_status - Data format for the
 * CHERRY_CORE_EVENT_TYPE_DEVICE_STATUS event.
 */
struct cherry_core_event_device_status {
	/**
	 * @state: The new state of the UWBS.
	 */
	enum cherry_core_device_state state;
	/**
	 * @reason: The boot reason of the UWBS.
	 */
	enum cherry_core_state_change_reason reason;
};

/**
 * struct cherry_core_event_device_error - Data format for the
 * CHERRY_CORE_EVENT_TYPE_ERROR event.
 */
struct cherry_core_event_device_error {
	/**
	 * @status_err: Indicate the error type. Will never be CHERRY_ERR_NONE
	 * as this is used to notified the upper layer of an error.
	 */
	enum cherry_err status_err;
};

/**
 * struct cherry_core_event_calib_update - Data format for the
 * CHERRY_CORE_EVENT_TYPE_CALIB_UPDATE event.
 */
struct cherry_core_event_calib_update {
	/**
	 * @status_err: CHERRY_ERR_NONE if the update is successful, otherwise
	 * see the cherry_err definition.
	 */
	enum cherry_err status_err;
};

/**
 * struct cherry_core_event_get_calib - Data format for the
 * CHERRY_CORE_EVENT_TYPE_GET_CALIB event.
 */
struct cherry_core_event_get_calib {
	/**
	 * @status_err: CHERRY_ERR_NONE if the request is successful, otherwise
	 * see the cherry_err definition.
	 */
	enum cherry_err status_err;
	/**
	 * @calib: Calibration values for the CHERRY_CORE_EVENT_TYPE_GET_CALIB
	 * event.
	 */
	struct cherry_calib *calib;
};

#define CHERRY_DEV_INFO_SOC_ID_LEN 32
#define CHERRY_DEV_INFO_FLAVOR_LEN 24

/**
 * struct cherry_core_event_device_info - Data format for the
 * CHERRY_CORE_EVENT_TYPE_DEVICE_INFO event.
 */
struct cherry_core_event_device_info {
	/**
	 * @status_err: CHERRY_ERR_NONE if the request is successful, otherwise
	 * see the cherry_err definition.
	 */
	enum cherry_err status_err;
	/**
	 * @uci_version: FiRa UCI version used by the UWBS.
	 * First Byte is the major version, minor is the other one
	 */
	uint16_t uci_version;
	/**
	 * @mac_version: FiRa MAC version used by the UWBS.
	 * First Byte is the major version, minor is the other one
	 */
	uint16_t mac_version;
	/**
	 * @phy_version: FiRa PHY version used by the UWBS.
	 * First Byte is the major version, minor is the other one
	 */
	uint16_t phy_version;
	/**
	 * @uci_test_version: FiRa UCI test specification used by the UWBS.
	 * First Byte is the major version, minor is the other one
	 */
	uint16_t uci_test_version;
	/**
	 * @fw_version: Null terminated string for the UWBS firmware version.
	 */
	char *fw_version;
	/**
	 * @soc_id: unique UWB chip id.
	 */
	uint8_t soc_id[CHERRY_DEV_INFO_SOC_ID_LEN];
	/**
	 * @device_id: UWBS HW device and revision ID
	 */
	uint32_t device_id;
	/**
	 * @package_id: 0 for SoC, 1 for SiP
	 */
	uint8_t package_id;
	/**
	 * @flavor: flavor name.
	 */
	uint8_t flavor[CHERRY_DEV_INFO_FLAVOR_LEN + 1];
	/**
	 * @product_id: product ID.
	 */
	uint32_t product_id;
	/**
	 * @soi_variant: SOI variant.
	 */
	uint32_t soi_variant;
	/**
	 * @rom_revision: ROM code revision.
	 */
	uint16_t rom_revision;
};

/**
 * struct cherry_core_event_device_stats - Data format for the
 * CHERRY_CORE_EVENT_TYPE_DEVICE_STATS event.
 */
struct cherry_core_event_device_stats {
	/**
	 * @status_err: CHERRY_ERR_NONE if the request is successful, otherwise
	 * see the cherry_err definition.
	 */
	enum cherry_err status_err;
	/**
	 * @temperature_100th_celsius: Temperature in 1/100th of Celsius degree.
	 * This is the temperature of the UWB chip.
	 */
	int16_t temperature_100th_celsius;
};

struct cherry_fira_capabilities;
struct cherry_ccc_capabilities;
struct cherry_radar_capabilities;

/**
 * struct cherry_core_event_device_capabilities - Data format for the
 * CHERRY_CORE_EVENT_TYPE_DEVICE_CAPS event.
 */
struct cherry_core_event_device_capabilities {
	/**
	 * @status_err: CHERRY_ERR_NONE if the request is successful, otherwise
	 * see the cherry_err definition.
	 */
	enum cherry_err status_err;
	/**
	 * @fira_capabilities: Device Fira capabilities.
	 */
	struct cherry_fira_capabilities *fira_capabilities;
	/**
	 * @ccc_capabilities: Device CCC capabilities.
	 */
	struct cherry_ccc_capabilities *ccc_capabilities;
	/**
	 * @radar_capabilities: Device Radar capabilities.
	 */
	struct cherry_radar_capabilities *radar_capabilities;
};

/**
 * struct cherry_core_event_device_timestamp - Data format for the
 * CHERRY_CORE_EVENT_TYPE_TIMESTAMP event.
 */
struct cherry_core_event_device_timestamp {
	/**
	 * @status_err: CHERRY_ERR_NONE if the request is successful, otherwise
	 * see the cherry_err definition.
	 */
	enum cherry_err status_err;
	/**
	 * @timestamp_us: Timestamp in the UWBS time base in microseconds.
	 */
	uint64_t timestamp_us;
};

/**
 * struct cherry_core_event_gpio_toggle - Data format for
 * CHERRY_CORE_EVENT_TYPE_GPIO_TOGGLE event.
 */
struct cherry_core_event_gpio_toggle {
	/**
	 * @status_err: CHERRY_ERR_NONE if the request is successful, otherwise
	 * see the cherry_err definition.
	 */
	enum cherry_err status_err;
	/**
	 * @timestamp_us: Timestamp.
	 *
	 * It coresponds to the GPIO toggling time in the UWBS time base in microseconds.
	 */
	uint64_t timestamp_us;
};

/**
 * struct cherry_core_event - Cherry core event message.
 *
 * Used by cherry to notify the upper layer through the callback define during
 * cherry_create.
 */
struct cherry_core_event {
	/**
	 * @type: Event that occurred.
	 */
	enum cherry_core_event_type type;
	/**
	 * @data: Pointer to the event data. Each event as it own data format
	 * and size.
	 */
	union {
		/**
		 * @data.device_status: Data for the
		 * CHERRY_CORE_EVENT_TYPE_DEVICE_STATUS event.
		 */
		struct cherry_core_event_device_status *device_status;
		/**
		 * @data.device_error: Data for the CHERRY_CORE_EVENT_TYPE_ERROR event.
		 */
		struct cherry_core_event_device_error *device_error;
		/**
		 * @data.device_info: Data for the CHERRY_CORE_EVENT_TYPE_DEVICE_INFO
		 * event.
		 */
		struct cherry_core_event_device_info *device_info;
		/**
		 * @data.device_stats: Data for the CHERRY_CORE_EVENT_TYPE_DEVICE_STATS
		 * event.
		 */
		struct cherry_core_event_device_stats *device_stats;
		/**
		 * @data.calib_update: Data for the CHERRY_CORE_EVENT_TYPE_CALIB_UPDATE
		 * event.
		 */
		struct cherry_core_event_calib_update *calib_update;
		/**
		 * @data.get_calib: Data for the CHERRY_CORE_EVENT_TYPE_GET_CALIB
		 * event.
		 */
		struct cherry_core_event_get_calib *get_calib;
		/**
		 * @data.device_caps: Data for the CHERRY_CORE_EVENT_TYPE_DEVICE_CAPS
		 * event.
		 */
		struct cherry_core_event_device_capabilities *device_caps;
		/**
		 * @data.device_timestamp: Data for the CHERRY_CORE_EVENT_TYPE_TIMESTAMP event.
		 */
		struct cherry_core_event_device_timestamp *device_timestamp;
		/**
		 * @data.gpio_toggle: Data for the CHERRY_CORE_EVENT_TYPE_GPIO_TOGGLE event.
		 */
		struct cherry_core_event_gpio_toggle *gpio_toggle;
	} data;
};

/**
 * typedef cherry_core_cb_t - Type for Cherry core notification callback.
 * @event: The event type and data.
 * @user_data: Pointer to client data defined at registration.
 *
 * The event is allocated by cherry and has to be freed by the application using
 * cherry_core_event_free().
 *
 * The callback implementation should return immediately and should not be blocking.
 */
typedef void (*cherry_core_cb_t)(struct cherry_core_event *event,
				 void *user_data);

/**
 * cherry_core_event_free() - Release the core event allocated memory.
 * @event: Pointer to the event to be freed.
 *
 * After each call to cherry_core_cb_t, this function has to be used to free the
 * corresponding event data. It lets the upper layer decide either to fully handle the
 * event inside the callback or to defer some processing outside without
 * temporary data copy.
 */
void cherry_core_event_free(struct cherry_core_event *event);

/**
 * cherry_create() - Initialize Cherry context.
 * @device: Path to UCI char dev in the file system.
 * @core_cb: Callback function to be called for core events.
 * @user_data: User data given at callback invocations.
 *
 * First Cherry API to be called. It provides the Cherry context needed by
 * other APIs.
 *
 * The core event CHERRY_CORE_EVENT_TYPE_DEVICE_STATUS will be emitted when
 * ready to handle further request. Or CHERRY_CORE_EVENT_TYPE_ERROR in case
 * issue.
 *
 * This function returns immediately.
 *
 * Returns: Newly allocated Cherry context or NULL in case of error.
 */
struct cherry *cherry_create(const char *device, cherry_core_cb_t core_cb,
			     void *user_data);

/**
 * cherry_destroy_sync() - Release and free Cherry context in synchronous way.
 * @ctx: Cherry context.
 *
 * Close the Cherry context and release internal memory. All session are forcibly closed and
 * `cherry_fira_session_destroy` must be called to free those pointers. It's recommended to first
 * stop any running session before calling this function because after it will not be possible to
 * stop them anymore on UWBS.
 * This is done synchronously.
 */
void cherry_destroy_sync(struct cherry *ctx);

/**
 * cherry_reset_device() - Reset the UWBS and cherry context.
 * @ctx: Cherry context.
 * @hard: Do hardware reset when true, otherwise do reset over UCI
 *
 * Request the reset of UWBS and cherry context.
 *
 * This function returns immediately.
 *
 * Event CHERRY_CORE_DEVICE_STATE_READY will be issue when the UWBS will get
 * back to normal.
 *
 * Returns:
 *  - &CHERRY_ERR_NONE if succeed
 *  - &CHERRY_ERR_INTERNAL for other errors
 */
enum cherry_err cherry_reset_device(struct cherry *ctx, bool hard);

/**
 * cherry_get_device_info() - Get the UWBS information.
 * @ctx: Cherry context.
 *
 * Request the UWBS device information like firwmare version and HW IDs.
 *
 * This function returns immediately.
 *
 * The device information are return with the core event
 * CHERRY_CORE_EVENT_TYPE_DEVICE_INFO.
 *
 * Returns:
 *  - &CHERRY_ERR_NONE if succeed
 *  - &CHERRY_ERR_INTERNAL for other errors
 */
enum cherry_err cherry_get_device_info(struct cherry *ctx);

/**
 * cherry_get_device_capabilities() - Get the UWBS capabilities.
 * @ctx: Cherry context.
 *
 * Request the UWBS capabilities which will be returned through the event
 * CHERRY_CORE_EVENT_TYPE_DEVICE_CAPS. For a comprehensive list of returned
 * capabilities refer to &cherry_core_event_device_capabilities.
 *
 * This function returns immediately.
 *
 * Returns:
 *  - &CHERRY_ERR_NONE if succeed
 *  - &CHERRY_ERR_INTERNAL for other errors
 */
enum cherry_err cherry_get_device_capabilities(struct cherry *ctx);

/**
 * cherry_get_device_timestamp() - Get timestamp in the UWBS time base.
 * @ctx: Cherry context.
 *
 * Request a timestamp in UWBS time base which will be returned through the event
 * CHERRY_CORE_EVENT_TYPE_TIMESTAMP.
 *
 * This function returns immediately.
 *
 * Returns:
 *  - &CHERRY_ERR_NONE if succeed
 *  - &CHERRY_ERR_INTERNAL for other errors
 */
enum cherry_err cherry_get_device_timestamp(struct cherry *ctx);

/**
 * CHERRY_CALIB_NUMBER() - Helper to create calibration number entry.
 * @Keyname: C string pointer to key name.
 * @Size: Size in bytes of number: 1, 2, 4.
 * @Number: Number value.
 */
#define CHERRY_CALIB_NUMBER(Keyname, Size, Number)                  \
	{                                                           \
		.name = Keyname, .type = CHERRY_CALIB_VALUE_NUMBER, \
		.size = Size, .number = Number                      \
	}

/**
 * CHERRY_CALIB_INT8() - Helper to create calibration int8_t entry
 * @Keyname: C string pointer to key name.
 * @Number: int8_t value.
 */
#define CHERRY_CALIB_INT8(Keyname, Number) \
	CHERRY_CALIB_NUMBER(Keyname, sizeof(int8_t), Number)

/**
 * CHERRY_CALIB_UINT8() - Helper to create calibration uint8_t entry
 * @Keyname: C string pointer to key name.
 * @Number: uint8_t value.
 */
#define CHERRY_CALIB_UINT8(Keyname, Number) CHERRY_CALIB_INT8(Keyname, Number)

/**
 * CHERRY_CALIB_INT16() - Helper to create calibration int16_t entry
 * @Keyname: C string pointer to key name.
 * @Number: int16_t value.
 */
#define CHERRY_CALIB_INT16(Keyname, Number) \
	CHERRY_CALIB_NUMBER(Keyname, sizeof(int16_t), Number)

/**
 * CHERRY_CALIB_UINT16() - Helper to create calibration uint16_t entry
 * @Keyname: C string pointer to key name.
 * @Number: uint16_t value.
 */
#define CHERRY_CALIB_UINT16(Keyname, Number) CHERRY_CALIB_INT16(Keyname, Number)

/**
 * CHERRY_CALIB_INT32() - Helper to create calibration int32_t entry
 * @Keyname: C string pointer to key name.
 * @Number: int32_t value.
 */
#define CHERRY_CALIB_INT32(Keyname, Number) \
	CHERRY_CALIB_NUMBER(Keyname, sizeof(int32_t), Number)

/**
 * CHERRY_CALIB_UINT32() - Helper to create calibration uint32_t entry
 * @Keyname: C string pointer to key name.
 * @Number: uint32_t value.
 */
#define CHERRY_CALIB_UINT32(Keyname, Number) \
	CHERRY_CALIB_INT32(Keyname, (int32_t)Number)

/**
 * CHERRY_CALIB_BOOL() - Helper to create calibration boolean entry
 * @Keyname: C string pointer to key name.
 * @Value: boolean value.
 */
#define CHERRY_CALIB_BOOL(Keyname, Value) CHERRY_CALIB_INT8(Keyname, Value)

/**
 * CHERRY_CALIB_NUMBER_ARRAY() - Helper to create calibration number array entry
 * @Keyname: C string pointer to key name.
 * @Arr: array pointer.
 * @NbItems: number of items in array. For multidimensional arrays it is the
 *           total of items from all dimensions. For example for an array of
 *           2 dimensions with 2 entries in first dimension and 8 in second
 *           dimension then NbItems = (2*8) = 16.
 */
#define CHERRY_CALIB_NUMBER_ARRAY(Keyname, Arr, NbItems)                    \
	{                                                                   \
		.name = Keyname, .type = CHERRY_CALIB_VALUE_NUMBER_ARRAY,   \
		.size = sizeof(Arr), .nb_array_items = NbItems, .data = Arr \
	}

/**
 * CHERRY_CALIB_NUMBER_ARRAY_1D() - Helper to create calibration number array
 * entry for 1 dimension arrays.
 * @Keyname: C string pointer to key name.
 * @Arr: array pointer.
 */
#define CHERRY_CALIB_NUMBER_ARRAY_1D(Keyname, Arr) \
	CHERRY_CALIB_NUMBER_ARRAY(Keyname, Arr, sizeof(Arr) / sizeof(Arr[0]))

/**
 * CHERRY_CALIB_NUMBER_ARRAY_2D() - Helper to create calibration number array
 * entry for 2 dimension arrays.
 * @Keyname: C string pointer to key name.
 * @Arr: array pointer.
 */
#define CHERRY_CALIB_NUMBER_ARRAY_2D(Keyname, Arr) \
	CHERRY_CALIB_NUMBER_ARRAY(Keyname, Arr, sizeof(Arr) / sizeof(Arr[0][0]))

enum cherry_calib_value_type {
	/**
	 * @CHERRY_CALIB_VALUE_NUMBER: value type is an integer number or a Q notation
	 */
	CHERRY_CALIB_VALUE_NUMBER,
	/**
	 * @CHERRY_CALIB_VALUE_NUMBER_ARRAY: value type is an array of integer or Q notation numbers.
	 */
	CHERRY_CALIB_VALUE_NUMBER_ARRAY,
	/**
	 * @CHERRY_CALIB_VALUE_DATA: value type is a buffer or a string.
	 */
	CHERRY_CALIB_VALUE_DATA,
};

struct cherry_calib_key {
	/**
	 * @name: calibration key name.
	 */
	const char *name;
	/**
	 * @type: type of value.
	 */
	enum cherry_calib_value_type type;
	/**
	 * @size: data size in bytes, even for single numbers. Number array must
	 * specify the size of the array in bytes.
	 */
	uint8_t size;
	/**
	 * @nb_array_items: numbers of items in array provided as data. Only applicable with
	 * CHERRY_CALIB_VALUE_NUMBER_ARRAY.
	 */
	uint8_t nb_array_items;
	union {
		/**
		 * @number: used when value is a number.
		 */
		int32_t number;
		/**
		 * @data: used when value is binary, a string, or a number array.
		 */
		const void *data;
	};
};

struct cherry_calib {
	/**
	 * @n_keys: number of keys to be set.
	 */
	uint32_t n_keys;
	/**
	 * @keys: Array of n_keys calibration keys.
	 */
	const struct cherry_calib_key *keys;
};

/**
 * cherry_set_calib() - Set the given calibration keys in device memory and on
 * device (re)connection.
 * @ctx: Cherry context.
 * @calib: calibration keys and values. Pointer and content must remain valid for the
 * duration of the cherry context or until cherry_set_calib(ctx, NULL) is called.
 * Calibration data can be accessed at any moment by cherry.
 *
 * The event CHERRY_CORE_EVENT_TYPE_CALIB_UPDATE will be issue when the UWBS
 * will have respond to the request. The status_err, in the event data, will
 * indicate if it succeed or not.
 *
 * Returns:
 *  - &CHERRY_ERR_NONE if succeed
 *  - &CHERRY_ERR_INTERNAL for other errors
 */
enum cherry_err cherry_set_calib(struct cherry *ctx,
				 const struct cherry_calib *calib);

/**
 * cherry_get_calib() - Get calibration keys from device memory.
 * @ctx: Cherry context.
 * @keys: Array of keys. Set to NULL to retrieve all calibration keys. Pointer and content
 * must remain valid for the duration of this API call until receiving the core event
 * CHERRY_CORE_EVENT_TYPE_GET_CALIB.
 * @n_keys: Keys array size.
 *
 * The calibration parameter currently used are return with the core event
 * CHERRY_CORE_EVENT_TYPE_GET_CALIB.
 *
 * Returns:
 *  - &CHERRY_ERR_NONE if succeed
 *  - &CHERRY_ERR_INVALID_PARAMETER if invalid parameters are provided.
 *  - &CHERRY_ERR_INTERNAL for other errors
 */
enum cherry_err cherry_get_calib(struct cherry *ctx, const char **keys,
				 const uint16_t n_keys);

/**
 * cherry_calib_destroy() - Destroy a calibration pointer returned by cherry. Calibration pointer
 * must be released using cherry_set_calib(ctx, NULL) before calling cherry_calib_folder_close().
 *
 * @calib: calibration information loaded with cherry_calib_open.
 */
void cherry_calib_destroy(struct cherry_calib *calib);

/**
 * cherry_get_device_stats() - Get UWB device stats.
 * @ctx: Cherry context.
 *
 * Request the UWBS device stats like chip temperature.
 *
 * This function returns immediately.
 *
 * The device information are return with the core event
 * CHERRY_CORE_EVENT_TYPE_DEVICE_STATS.
 *
 * Returns:
 *  - &CHERRY_ERR_NONE if succeed
 *  - &CHERRY_ERR_INTERNAL for other errors
 */
enum cherry_err cherry_get_device_stats(struct cherry *ctx);

/**
  * enum cherry_log_level - Enumerate log levels.
  * @CHERRY_LOG_LEVEL_NONE: No log.
  * @CHERRY_LOG_LEVEL_ERROR: Error.
  * @CHERRY_LOG_LEVEL_WARN: Warning.
  * @CHERRY_LOG_LEVEL_INFO: Info.
  * @CHERRY_LOG_LEVEL_DEBUG: Debug.
  */
enum cherry_log_level {
	CHERRY_LOG_LEVEL_NONE,
	CHERRY_LOG_LEVEL_ERROR,
	CHERRY_LOG_LEVEL_WARN,
	CHERRY_LOG_LEVEL_INFO,
	CHERRY_LOG_LEVEL_DEBUG,
};

/**
  * enum cherry_log_module - Enumerate log modules.
  * @CHERRY_LOG_MODULE_ALL: Apply log level change to all module including UWBS and Cherry
  */
enum cherry_log_module {
	CHERRY_LOG_MODULE_ALL = (int)0xFFFFFFFF,
};

/**
 * cherry_set_log_level() - Change the log level.
 * @ctx: Cherry context.
 * @level: Level of log.
 * @module: Module on which the log level change has to be applied.
 *
 * Configure the log level on selected module. In case of issue, no error event will
 * be reported to the app. Only an error log will be provided by cherry.
 *
 * This function returns immediately.
 *
 * Returns:
 *  - &CHERRY_ERR_NONE if succeed
 *  - &CHERRY_ERR_INTERNAL for other errors
 */
enum cherry_err cherry_set_log_level(struct cherry *ctx,
				     enum cherry_log_level level,
				     enum cherry_log_module module);

/**
 * cherry_toggle_gpio_time_sync() - Request GPIO toggle and timestamp from UWBS.
 * @ctx: Cherry context.
 * @mode: GPIO mode to use.
 *
 * Request GPIO toggle and a timestamp from UWBS.
 *
 * This function returns immediately.
 *
 * Returns:
 *  - &CHERRY_ERR_NONE if accepted
 *  - &CHERRY_ERR_INVALID_PARAMETER on any invalid parameter
 */
enum cherry_err
cherry_toggle_gpio_time_sync(struct cherry *ctx,
			     enum cherry_core_gpio_toggle_mode mode);

/**
 * cherry_err_str() - Get cherry error string.
 * @err: The cherry error.
 *
 * Returns: A error string.
 */
const char *cherry_err_str(enum cherry_err err);

#ifdef __cplusplus
}
#endif
