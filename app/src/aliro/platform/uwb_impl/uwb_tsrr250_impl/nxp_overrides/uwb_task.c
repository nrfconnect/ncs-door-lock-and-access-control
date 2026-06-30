
/*
 *
 * Copyright 2018-2024 NXP.
 *
 * NXP Proprietary. This software is owned or controlled by NXP and may only be
 * used strictly in accordance with the applicable license terms. By expressly
 * accepting such terms or by downloading,installing, activating and/or otherwise
 * using the software, you are agreeing that you have read,and that you agree to
 * comply with and are bound by, such license terms. If you do not agree to be
 * bound by the applicable license terms, then you may not retain, install, activate
 * or otherwise use the software.
 *
 */

/**  Entry point for UWB_TASK */
#include "UwbAdaptation.h"
#include "uwb_target.h"
#include "uwb_hal_api.h"
#include "uwb_int.h"
#include "uci_hmsgs.h"

#include "uwa_sys.h"
#include "uwa_dm_int.h"
#include "phNxpLogApis_UciCore.h"
#include <stdio.h>

phUwbtask_Control_t *gp_uwbtask_ctrl;

static uint32_t gTimerId = PH_OSALUWB_TIMER_ID_INVALID;

/** Local functions prototypes */
static void quick_timer_callback(uint32_t timerid, void *pContext);
static void release_quick_timer(void);

static void quick_timer_callback(uint32_t timerid, void *pContext)
{
    static uint16_t quickEvent = UWB_TTYPE_UCI_WAIT_RSP;

    phUwb_OSAL_send_msg(UWB_TASK, TIMER_1_EVT_MASK, &quickEvent);
}

void uwb_start_quick_timer(uint32_t timeout)
{
    UCI_TRACE_D("uwb_start_quick_timer enter: timeout: %dms", timeout);

    if (gTimerId == PH_OSALUWB_TIMER_ID_INVALID) {
        gTimerId = phOsalUwb_Timer_Create(FALSE);
    }

    if (gTimerId != PH_OSALUWB_TIMER_ID_INVALID) {
        (void)phOsalUwb_Timer_Stop(gTimerId);
        if (phOsalUwb_Timer_Start(gTimerId, timeout, &quick_timer_callback, NULL) != UWBSTATUS_SUCCESS) {
            UCI_TRACE_E("%s: Timer Start failed with Timer ID %d", __FUNCTION__, gTimerId);
        }
    }
    else {
        UCI_TRACE_E("%s : Invalid Timer ID %d", __FUNCTION__, gTimerId);
    }
}

void uwb_stop_quick_timer()
{
    UCI_TRACE_D("uwb_stop_quick_timer: enter");

    if (gTimerId == PH_OSALUWB_TIMER_ID_INVALID) {
        return;
    }

    if (phOsalUwb_Timer_Stop(gTimerId) != UWBSTATUS_SUCCESS) {
        UCI_TRACE_D("%s: Timer Stop failed with Timer ID %d", __FUNCTION__, gTimerId);
    }
}

static void release_quick_timer(void)
{
    if (gTimerId == PH_OSALUWB_TIMER_ID_INVALID) {
        return;
    }

    (void)phOsalUwb_Timer_Stop(gTimerId);
    if (phOsalUwb_Timer_Delete(gTimerId) != UWBSTATUS_SUCCESS) {
        UCI_TRACE_D("%s: Timer Delete failed with Timer ID %d", __FUNCTION__, gTimerId);
    }
    gTimerId = PH_OSALUWB_TIMER_ID_INVALID;
}

void uwb_process_quick_timer_evt(uint16_t event)
{
    switch (event) {
    case UWB_TTYPE_UCI_WAIT_RSP:
        if (!uwb_cb.is_resp_pending) {
            UCI_TRACE_D("Ignoring stale UCI response timer event");
            break;
        }
        uwb_ucif_cmd_timeout();
        break;

    default:
        UCI_TRACE_D("uwb_process_quick_timer_evt: event (0x%04x)", event);
    }
}

void uwb_task_shutdown_uwbc(void)
{
    release_quick_timer();
    uwb_gen_cleanup();

    uwb_set_state(UWB_STATE_W4_HAL_CLOSE);
    uwb_cb.p_hal->close();
}

#define UWB_TASK_ARGS void *args

#if UWBIOT_OS_FREERTOS
#define RET_VALUE return
#else
#define RET_VALUE return 0
#endif

OSAL_TASK_RETURN_TYPE uwb_task(UWB_TASK_ARGS)
{
    uint32_t event;
    bool free_buf  = FALSE;
    UWB_HDR *p_msg = NULL;
    phLibUwb_Message_t msg;
    gp_uwbtask_ctrl    = (phUwbtask_Control_t *)args;
    tUCI_STATUS status = UCI_STATUS_FAILED;

    if (gp_uwbtask_ctrl == NULL || gp_uwbtask_ctrl->pMsgQHandle == 0) {
        printf("[TSRR250][UWB_TASK] invalid startup context 0x%08lx\n",
               (unsigned long)(uintptr_t)gp_uwbtask_ctrl);
        return;
    }

    /* Initialize the message */
    phOsalUwb_SetMemory(&msg, 0, sizeof(msg));

    UCI_TRACE_D("UWB_TASK started.");

    /* main loop */
    while (1) {
        if (phOsalUwb_msgrcv(gp_uwbtask_ctrl->pMsgQHandle, &msg, MAX_DELAY) == UWBSTATUS_FAILED) {
            continue;
        }
        event = msg.eMsgType;
        /* Handle UWB_TASK_EVT_TRANSPORT_READY from UWB HAL */
        if (event & UWB_TASK_EVT_TRANSPORT_READY) {
            UCI_TRACE_D("UWB_TASK got UWB_TASK_EVT_TRANSPORT_READY.");

            /* Reset the UWB controller. */
            uwb_set_state(UWB_STATE_IDLE);
            status = UCI_STATUS_OK;
            if (uwa_dm_cb.p_dm_rsp_cback != NULL) {
                (*uwa_dm_cb.p_dm_rsp_cback)(UCI_GID_INTERNAL, UCI_ENABLE, sizeof(tUCI_STATUS), &status);
            }
            else {
                printf("[TSRR250][UWB_TASK] transport ready with null DM response callback\n");
            }
        }

        if (event & UWB_SHUTDOWN_EVT_MASK) {
            break;
        }

        if (event & UWB_MBOX_EVT_MASK) {
            /* Process all incoming UCI messages */
            p_msg    = (UWB_HDR *)msg.pMsgData;
            free_buf = TRUE;

            /* Determine the input message type. */
            if (p_msg != NULL) {
                switch (p_msg->event & UWB_EVT_MASK) {
                case BT_EVT_TO_UWB_UCI:
                    free_buf = uwb_ucif_process_event(p_msg);
                    break;

                case BT_EVT_TO_UWB_MSGS:
                    uwb_main_handle_hal_evt((tUWB_HAL_EVT_MSG *)p_msg);
                    break;

                default:
                    UCI_TRACE_E("uwb_task: unhandle mbox message, event=%04x", p_msg->event);
                    break;
                }
                if (free_buf) {
                    phOsalUwb_FreeMemory(p_msg);
                }
            }
        }

        /* Process quick timer tick */
        if (event & UWB_QUICK_TIMER_EVT_MASK) {
            if (msg.pMsgData != NULL) {
                uwb_process_quick_timer_evt(*((uint16_t *)msg.pMsgData));
            }
            else {
                printf("[TSRR250][UWB_TASK] quick timer event with null payload\n");
            }
        }

        if (event & UWA_MBOX_EVT_MASK) {
            uwa_sys_event(&(((tUWA_DM_API_ENABLE *)msg.pMsgData)->hdr));
        }
    }

    UCI_TRACE_D("uwb_task terminated");
    phOsalUwb_ProduceSemaphore(gp_uwbtask_ctrl->uwb_task_sem);

    /* Suspend task here so that it does not return in FreeRTOS
     * Task will be deleted in shutdown sequence
     */
    (void)phOsalUwb_TaskSuspend(gp_uwbtask_ctrl->task_handle);
}
