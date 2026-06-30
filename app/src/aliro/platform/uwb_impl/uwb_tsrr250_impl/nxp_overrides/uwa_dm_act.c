/**
 *
 *  Copyright (C) 1999-2014 Broadcom Corporation
 *  Copyright 2018-2024 NXP
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at:
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 */
/**
 *
 *  This file contains the action functions for device manager state
 *  machine.
 *
 */
#include "uwa_sys.h"
#include "uwa_api.h"
#include "uwa_dm_int.h"
#include "uci_hmsgs.h"
#include "phNxpLogApis_UciCore.h"
#include "uwb_int.h"
#include "uwb_hal_int.h"
#include <stdio.h>

bool uwa_dm_enable(tUWA_DM_MSG *p_data)
{
    UCI_TRACE_D("uwa_dm_enable ()");
    tUCI_STATUS status = UCI_STATUS_FAILED;
    /* Check if UWA is already enabled */
    if (!(uwa_dm_cb.flags & UWA_DM_FLAGS_DM_IS_ACTIVE)) {
        /* Store Enable parameters */
        uwa_dm_cb.p_dm_rsp_cback = p_data->enable.p_dm_rsp_cback;
        uwa_dm_cb.p_dm_ntf_cback = p_data->enable.p_dm_ntf_cback;
#if UWBFTR_DataTransfer
        uwa_dm_cb.p_dm_data_cback = p_data->enable.p_dm_data_cback;
#endif // UWBFTR_DataTransfer
        /* Enable UWB stack */
        uwb_set_state(UWB_STATE_W4_HAL_OPEN);
        uwb_cb.p_hal->open(&uwb_main_hal_cback, &uwb_main_hal_data_cback);
    }
    else {
        UCI_TRACE_E("uwa_dm_enable: ERROR ALREADY ENABLED.");

        (*(p_data->enable.p_dm_rsp_cback))(UCI_GID_INTERNAL, UCI_ENABLE, sizeof(tUCI_STATUS), &status);
    }

    return (TRUE);
}

bool uwa_dm_disable(tUWA_DM_MSG *p_data)
{
    UCI_TRACE_D("uwa_dm_disable (): graceful:%d", p_data->disable.graceful);

    /* Disable all subsystems other than DM (DM will be disabled after all  */
    /* the other subsystem have been disabled)                              */
    uwa_sys_disable_subsystems(p_data->disable.graceful);

    return (TRUE);
}

bool uwa_dm_register_ext_cb(tUWA_DM_MSG *p_data)
{
    UCI_TRACE_D("uwa_dm_register_ext_cb ()");

    /* Validate callback */
    if (!p_data->ext_cb.p_dm_ext_cback) {
        return (UCI_STATUS_INVALID_PARAM);
    }

    /* if UWA is enabled, then register for callback */
    if (uwa_dm_cb.flags & UWA_DM_FLAGS_DM_IS_ACTIVE) {
        uwb_cb.p_ext_resp_cback = p_data->ext_cb.p_dm_ext_cback;
        (*(uwa_dm_cb.p_dm_rsp_cback))(UCI_GID_INTERNAL, UCI_REG_EXT_CB, 0, NULL);
    }
    return (TRUE);
}

void uwa_dm_disable_complete(void)
{
    UCI_TRACE_D("uwa_dm_disable_complete ()");
    /* Disable uwb core stack */
    if (uwb_cb.uwb_state == UWB_STATE_NONE) {
        if (uwa_dm_cb.p_dm_rsp_cback != NULL) {
            (*uwa_dm_cb.p_dm_rsp_cback)(UCI_GID_INTERNAL, UCI_DISABLE, 0, NULL);
        }
        else {
            printf("[TSRR250][UCI] disable complete with null DM response callback\n");
        }
        uwa_dm_cb.p_dm_rsp_cback = NULL;
        uwa_dm_cb.p_dm_ntf_cback = NULL;
#if UWBFTR_DataTransfer
        uwa_dm_cb.p_dm_data_cback = NULL;
#endif // UWBFTR_DataTransfer
        return;
    }

    /* Close transport and clean up */
    uwb_task_shutdown_uwbc();
}

bool uwa_dm_act_send_raw_cmd(tUWA_DM_MSG *p_data)
{
    /* Validate parameters */
    if (p_data == NULL) {
        return UCI_STATUS_INVALID_PARAM;
    }

    UWB_HDR *p_cmd = (UWB_HDR *)p_data;

    p_cmd->offset = sizeof(tUWA_DM_API_SEND_RAW) - UWB_HDR_SIZE;
    p_cmd->len    = p_data->send_raw.cmd_params_len;

    p_cmd->event          = BT_EVT_TO_UWB_UCI;
    p_cmd->layer_specific = UWB_WAIT_RSP_RAW_CMD;
    /* save the callback function in the BT_HDR, to receive the response */
    ((tUWB_UCI_RAW_MSG *)p_cmd)->p_cback = p_data->send_raw.p_cback;

    uwb_ucif_check_cmd_queue(p_cmd);

    return FALSE;
}
