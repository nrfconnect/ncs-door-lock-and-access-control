/**
 * DOC: Interface for the Aliro UWB adapter.
 *
 * SPDX-FileCopyrightText: Copyright (c) 2025 Qorvo, Inc.
 * SPDX-License-Identifier: LicenseRef-Qorvo-SDK-1
 */

#pragma once

#include <cherry/cherry.h>
#include <cherry/cherry_ccc.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * define ALIRO_UWB_PREFERRED_HOP_CONFIG_MAX: Maximum number of preferred hopping configuration.
 */
#define ALIRO_UWB_ADAPTER_PREFERRED_HOP_CONFIG_MAX 3

/**
 * DOC: struct aliro_uwb_adapter
 *
 * Object representing the aliro_uwb_adapter.
 *
 * It is created by the aliro_uwb_adapter_create_reader() function and destroyed by
 * aliro_uwb_adapter_destroy(). It's an opaque struct to store the Aliro UWB adapter
 * data that are independent of any session.
 */
struct aliro_uwb_adapter;

/**
 * enum aliro_uwb_err - Status code returned by Aliro UWB adapter.
 */
enum aliro_uwb_err {
	/**
	 * @ALIRO_UWB_ERR_NONE: No error.
	 */
	ALIRO_UWB_ERR_NONE,
	/**
	 * @ALIRO_UWB_ERR_INVALID_PARAMETER: Invalid parameter.
	 */
	ALIRO_UWB_ERR_INVALID_PARAMETER,
	/**
	 * @ALIRO_UWB_ERR_UWBS_TIMEOUT: Didn't get the UWBS respond to an UCI command
	 * before the time limit.
	 */
	ALIRO_UWB_ERR_UWBS_TIMEOUT,
	/**
	 * @ALIRO_UWB_ERR_INTERNAL: An internal error occurred.
	 */
	ALIRO_UWB_ERR_INTERNAL,
	/**
	 * @ALIRO_UWB_ERR_SESSION_INIT: Initialization of the new session has
	 * failed.
	 */
	ALIRO_UWB_ERR_SESSION_INIT,
	/**
	 * @ALIRO_UWB_ERR_SESSION_ACTIVE: Operation can not be done on active
	 * session.
	 */
	ALIRO_UWB_ERR_SESSION_ACTIVE,
	/**
	 * @ALIRO_UWB_ERR_SESSION_CONFIG: The session can not be started because
	 * it's missing some configurations.
	 */
	ALIRO_UWB_ERR_SESSION_CONFIG,
	/**
	 * @ALIRO_UWB_ERR_MESSAGE_UNSUPPORTED: The Aliro UWB adapter does not support the provided
	 * Aliro message.
	 */
	ALIRO_UWB_ERR_MESSAGE_UNSUPPORTED,
	/**
	 * @ALIRO_UWB_ERR_MESSAGE_STATE: The Aliro message provided is not supported by the
	 * Aliro UWB adapter.
	 */
	ALIRO_UWB_ERR_MESSAGE_STATE,
	/**
	 * @ALIRO_UWB_ERR_INVALID_STATE: The Aliro state is invalid.
	 */
	ALIRO_UWB_ERR_INVALID_STATE,
	/**
	 * @ALIRO_UWB_ERR_MSG_MALFORMED: The Aliro message provided is malformed.
	 */
	ALIRO_UWB_ERR_MSG_MALFORMED,
};

/**
 * enum aliro_hopping_config - Hopping configuration for Aliro UWB.
 */
enum aliro_hopping_config {
	/**
	 * @ALIRO_HOPPING_CONFIG_DISABLED: Hopping is disabled.
	 */
	ALIRO_HOPPING_CONFIG_DISABLED = CHERRY_CCC_HOPPING_MODE_DISABLE,
	/**
	 * @ALIRO_HOPPING_CONFIG_CONTINUOUS_DEFAULT: Continuous hopping with default sequence.
	 */
	ALIRO_HOPPING_CONFIG_CONTINUOUS_DEFAULT = CHERRY_CCC_HOPPING_MODE_CONTINUOUS_DEFAULT,
	/**
	 * @ALIRO_HOPPING_CONFIG_ADAPTIVE_DEFAULT: Adaptive hopping with default sequence.
	 */
	ALIRO_HOPPING_CONFIG_ADAPTIVE_DEFAULT = CHERRY_CCC_HOPPING_MODE_ADAPTATIVE_DEFAULT,
};

/**
 * struct aliro_uwb_preferred_hopping_configs - Preferred hopping configurations for Aliro UWB.
 */
struct aliro_uwb_preferred_hopping_configs {
	/**
	 * @configs: Array of preferred hopping configurations. This list must at least contain
	 * one hopping configuration using the default hopping sequence:
	 * ALIRO_HOPPING_CONFIG_CONTINUOUS_DEFAULT or ALIRO_HOPPING_CONFIG_ADAPTIVE_DEFAULT as it is
	 * mandatory according to Aliro specification.
	 */
	enum aliro_hopping_config configs[ALIRO_UWB_ADAPTER_PREFERRED_HOP_CONFIG_MAX];
	/**
	 * @count: Number of preferred hopping configurations.
	 */
	size_t count;
};

/**
 * struct aliro_uwb_adapter_reader_config - Configuration parameters for Aliro UWB reader.
 *
 * The Aliro reader device behaves as a UWB Responder. It selects some of the UWB session parameters
 * based on its capabilities, the user device's capabilities, and some preferred values that can be
 * configured through this structure.
 *
 * Below is the list of parameters that must be selected by the reader device according to the
 * Aliro specification and how the Aliro UWB adapter selects each of them:
 *  - UWB_Session_Identifier: Selected by the Aliro UWB adapter at session creation.
 *  - Session_RAN_Multiplier: Selects the largest value among the one supported by the user device,
 *    the reader's UWB capabilities, and the one set in @min_ran_multiplier.
 *  - Number_Chaps_per_Slot: Selects the smallest value supported by both the reader and the user
 *    device.
 *  - Selected_Hopping_Config_Bitmask: Selects the first element of @preferred_hopping_config
 *    supported by both the user device and the reader's UWB capabilities.
 *  - Number_Slots_per_Round: Fixed to the default value (12).
 *  - Number_Responder_Nodes: Always set to 1, as the Aliro UWB adapter supports only one responder
 *    node.
 *  - Mac_mode: Set to the value of @mac_mode.
 */
struct aliro_uwb_adapter_reader_config {
	/**
	 * @min_ran_multiplier: Minimum RAN multiplier.
	 *
	 * The RAN mulitplier allows to define the time between 2 ranging blocks:
	 *  - T_Block RAN = RAN_Multiplier × 96 ms
	 *
	 * The time range is between 96 ms (RAN mulitplier is 1) and 24480 ms (RAN multiplier
	 * is 255).
	 */
	uint8_t min_ran_multiplier;
	/**
	 * @preferred_hopping_config: Ordered list of preferred hopping configuration.
	 */
	struct aliro_uwb_preferred_hopping_configs preferred_hopping_configs;
	/**
	 * @mac_mode: Specify if one or multiple ranging round are used in a ranging block.
	 *
	 *  b0-b5: Offset between the 2 ranging ranging blocks.
	 *  b5-b6: Number of ranging round used: 0b00 -> 1 ranging round, 0b01 -> 2 ranging round.
	 */
	uint8_t mac_mode;
	/**
	 * @r1_antennas: Tx and Rx antenna set to be used for the 1st ranging round.
	 *
	 *  r1_antennas[0]: Antenna set to be used as Tx for the 1st ranging round.
	 *  r1_antennas[1]: Antenna set to be used as Rx for the 1st ranging round.
	 *
	 *  By default, the value is set to 0.
	 */
	uint8_t r1_antennas[2];
	/**
	 * @r2_antennas: Tx and Rx antenna set to be used for the 2nd ranging round.
	 *
	 *  r2_antennas[0]: Antenna set to be used as Tx for the 2nd ranging round.
	 *  r2_antennas[1]: Antenna set to be used as Rx for the 2nd ranging round.
	 *
	 *  This parameter is used when Aliro MAC mode 1 is selected.
	 */
	uint8_t r2_antennas[2];
};

/**
 * aliro_uwb_adapter_create_reader() - Create the aliro_uwb_adapter for a Reader device.
 * @cherry_ctx: Cherry context.
 * @caps: UWBS capabilities.
 * @config: Configuration for the Aliro UWB adapter.
 *
 * This function initializes and creates the Aliro UWB adapter in responder mode.
 * It is the first API to be called to set up the adapter context.
 *
 * The cherry_create function allows to get the cherry_ctx and cherry_get_device_capabilities the
 * pointer to device capabilities.
 *
 * When no more needed, the adapter context must be destroyed using aliro_uwb_adapter_destroy().
 *
 * The @config pointer must remain valid until the adapter context is destroyed. Modifying
 * any parameters in the @config structure after the adapter context is created is prohibited
 * and may lead to undefined behavior.
 *
 * Return: Pointer to the created adapter context on success, or NULL if the
 * initialization fails due to invalid parameters or internal errors.
 */
struct aliro_uwb_adapter *
aliro_uwb_adapter_create_reader(struct cherry *cherry_ctx,
				struct cherry_core_event_device_capabilities *caps,
				struct aliro_uwb_adapter_reader_config *config);

/**
 * aliro_uwb_adapter_set_diagnostics() - Enable or disable diagnostics reporting for all
 * sessions.
 * @aliro_ctx: Aliro UWB context.
 * @config: Define which kind of diagnostics to enable.
 *
 * The new diagnostics configuration will be applied to all new sessions created after this call.
 */
void aliro_uwb_adapter_set_diagnostics(struct aliro_uwb_adapter *aliro_ctx,
				       struct cherry_common_diag_cfg config);

/**
 * aliro_uwb_adapter_destroy() - Destroy the Aliro UWB adapter.
 * @aliro_ctx: Aliro UWB context.
 *
 * Close the Aliro UWB adapter and release context and internal memory.
 * The &aliro_ctx can no longer be used after this call.
 */
void aliro_uwb_adapter_destroy(struct aliro_uwb_adapter *aliro_ctx);

#ifdef __cplusplus
}
#endif
