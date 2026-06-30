/**
 *
 *  Copyright (C) 1999-2012 Broadcom Corporation
 *  Copyright 2018-2019,2022-2024 NXP
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

#include "UwbAdaptation.h"
#include "UwbCoreSDK_Internal.h"
#include "phNxpUciHal_Adaptation.h"
#include "phNxpUwbConfig.h"
#include "phOsalUwb.h"
#include "uwa_api.h"
#include "uwb_hal_int.h"
#include "uwb_int.h"
#include "uwb_logging.h"
#include "uwb_target.h"
#include "phUwb_BuildConfig.h"
#include "phNxpLogApis_UwbApi.h"
#include <stdio.h>

#define MAX_TIMEOUT_UWB_TASK_SEM (100)

static phUwbtask_Control_t uwb_ctrl;

static void HalOpen(tHAL_UWB_CBACK *p_hal_cback, tHAL_UWB_DATA_CBACK *p_data_cback);
static void HalClose(void);
static void HalWrite(uint16_t data_len, uint8_t *p_data);
static tUCI_STATUS HalIoctl(long arg, tHAL_UWB_IOCTL *p_data);

static const tHAL_UWB_ENTRY mHalEntryFuncs = {&HalOpen, &HalClose, &HalWrite, &HalIoctl};

extern phUwbtask_Control_t *gp_uwbtask_ctrl;

#define TIMER_1_EVT_MASK 0x0020

uint32_t StartUwbTask()
{
    phOsalUwb_ThreadCreationParams_t threadparams;
    UWBIOT_STACK_DEFINE(UWB_TASK_stack, UWBTASK_STACK_SIZE);

    PHOSALUWB_SET_TASKNAME(threadparams, "UWB_TASK");
    threadparams.pContext = &uwb_ctrl;
#if UWBIOT_OS_ZEPHYR
    threadparams.priority = 1;
#else
    threadparams.priority = 5;
#endif
#if UWBIOT_OS_ZEPHYR
    threadparams.pStack = (k_thread_stack_t *)&UWB_TASK_stack;
    threadparams.stackdepth = UWBIOT_THREAD_STACK_SIZE(UWB_TASK_stack);
#else
    threadparams.stackdepth = UWBTASK_STACK_SIZE;
#endif // UWBIOT_OS_ZEPHYR
    if (phOsalUwb_Thread_Create((void **)&uwb_ctrl.task_handle, &uwb_task, &threadparams) != 0) {
        return UCI_STATUS_FAILED;
    }
    return UCI_STATUS_OK;
}

void phUwb_OSAL_send_msg(uint8_t task_id, uint16_t mbox, void *pmsg)
{
    phLibUwb_Message_t msg;
    intptr_t pMsgQ = 0;

    phOsalUwb_SetMemory(&msg, 0, sizeof(msg));

    switch (mbox) {
    case TIMER_1_EVT_MASK:
    case UWB_TASK_EVT_TRANSPORT_READY:
    case UWB_SHUTDOWN_EVT_MASK:
        msg.eMsgType = (uint16_t)mbox;
        break;
    default:
        msg.eMsgType = (uint16_t)UWB_EVENT_MASK(mbox);
    }

    msg.pMsgData = pmsg;
    msg.Size     = 0;

    if (task_id == UWB_TASK) {
        pMsgQ = uwb_ctrl.pMsgQHandle;
    }

    if (pMsgQ == 0) {
        printf("[TSRR250][UWB] drop mbox 0x%04x: UWB task queue is not ready\n", mbox);
        return;
    }

    if (phOsalUwb_msgsnd(pMsgQ, &msg, NO_DELAY) != UWBSTATUS_SUCCESS) {
        printf("[TSRR250][UWB] failed to post mbox 0x%04x to queue 0x%08lx\n",
               mbox, (unsigned long)(uintptr_t)pMsgQ);
    }
}

tUCI_STATUS Initialize(void)
{
    LOG_D("%s: Enter", __FUNCTION__);
    uint8_t mConfig;
    tUCI_STATUS status = UCI_STATUS_OK;

    uwb_ctrl.pMsgQHandle = phOsalUwb_msgget(configTML_QUEUE_LENGTH);
    if (uwb_ctrl.pMsgQHandle == (intptr_t)NULL) {
        LOG_E("%s: memory allocation for pMsgQHandle Failed", __FUNCTION__);
        status = UCI_STATUS_FAILED;
        goto exit;
    }

    gp_uwbtask_ctrl = &uwb_ctrl;

    status = (tUCI_STATUS)phOsalUwb_CreateSemaphore(&uwb_ctrl.uwb_task_sem, 0);
    if (status != UWBSTATUS_SUCCESS) {
        LOG_E("%s: uwb_task_sem creation Failed", __FUNCTION__);
        status = UCI_STATUS_FAILED;
        goto exit;
    }

    status = StartUwbTask();
    if (status == UCI_STATUS_OK) {
        (void)phNxpUciHal_GetNxpNumValue(UWB_FW_LOG_THREAD_ID, &mConfig, sizeof(mConfig));
    }
    else {
        status = UCI_STATUS_FAILED;
        LOG_E("%s: UWB-Task creation Failed", __FUNCTION__);
        goto exit;
    }

exit:
    if (status != UCI_STATUS_OK) {
        /** Gracefully release the allocated memory */
        phOsalUwb_msgrelease(uwb_ctrl.pMsgQHandle);
        uwb_ctrl.pMsgQHandle = 0;
        gp_uwbtask_ctrl = NULL;
        /** Delete the semaphore */
        phOsalUwb_DeleteSemaphore(&uwb_ctrl.uwb_task_sem);
    }

    LOG_D("%s: Exit", __FUNCTION__);
    return status;
}

void Finalize()
{
    if (uwb_ctrl.pMsgQHandle == 0) {
        return;
    }

    phUwb_OSAL_send_msg(UWB_TASK, UWB_SHUTDOWN_EVT_MASK, NULL);
    if (UWBSTATUS_SUCCESS != phOsalUwb_ConsumeSemaphore_WithTimeout(uwb_ctrl.uwb_task_sem, MAX_TIMEOUT_UWB_TASK_SEM)) {
        LOG_E("%s : phOsalUwb_ConsumeSemaphore_WithTimeout failed", __FUNCTION__);
    }
    phOsalUwb_DeleteSemaphore(&uwb_ctrl.uwb_task_sem);
    phOsalUwb_msgrelease(uwb_ctrl.pMsgQHandle);
    uwb_ctrl.pMsgQHandle = 0;
    gp_uwbtask_ctrl = NULL;
    (void)phOsalUwb_Thread_Delete(uwb_ctrl.task_handle);
}

const tHAL_UWB_ENTRY *GetHalEntryFuncs()
{
    return &mHalEntryFuncs;
}

/**
**
** Function:    HalOpen
**
** Description: Turn on controller, download firmware.
**
** Returns:     None.
**
*/
static void HalOpen(ATTRIBUTE_UNUSED tHAL_UWB_CBACK *p_hal_cback, ATTRIBUTE_UNUSED tHAL_UWB_DATA_CBACK *p_data_cback)
{
    (void)phNxpUciHal_open(p_hal_cback, p_data_cback);
}

/**
**
** Function:    HalClose
**
** Description: Turn off controller.
**
** Returns:     None.
**
*/
static void HalClose()
{
    (void)phNxpUciHal_close();
}

/**
**
** Function:    HalWrite
**
** Description: Write UCI message to the controller.
**
** Returns:     None.
**
*/
static void HalWrite(ATTRIBUTE_UNUSED uint16_t data_len, ATTRIBUTE_UNUSED uint8_t *p_data)
{
    (void)phNxpUciHal_write(data_len, p_data);
}

void HalRegisterAppCallback(phHalAppDataCb *recvDataCb)
{
    phNxpUciHal_register_appdata_callback(recvDataCb);
}

/**
**
** Function:    HalIoctl
**
** Description: Calls ioctl to the Uwb driver.
**              If called with a arg value of 0x01 than wired access requested,
**              status of the request would be updated to p_data.
**              If called with a arg value of 0x00 than wired access will be
**              released, status of the request would be updated to p_data.
**              If called with a arg value of 0x02 than current p61 state would
*be
**              updated to p_data.
**
** Returns:     -1 or 0.
**
*/
static tUCI_STATUS HalIoctl(ATTRIBUTE_UNUSED long arg, ATTRIBUTE_UNUSED tHAL_UWB_IOCTL *p_data)
{
    tUCI_STATUS status = 0;
    status             = (tUCI_STATUS)phNxpUciHal_ioctl(arg, p_data);
    return status;
}

tUCI_STATUS UwbDeviceInit(bool recovery)
{
    tUCI_STATUS status = 0;
    int temp_status;
    temp_status             = phNxpUciHal_uwbDeviceInit(recovery);
    if(temp_status != UWBSTATUS_SUCCESS){
        status = UWBSTATUS_FAILED;
    }
    else{
        status = UWBSTATUS_SUCCESS;
    }
    return status;
}

bool isCmdRespPending()
{
    return uwb_cb.is_resp_pending;
}

void Hal_setOperationMode(Uwb_operation_mode_t state)
{
    /* register the state of the user mode in Hal */
    phNxpUciHal_SetOperatingMode(state);
    /*register the operating mode in uwb context to use it in Api*/
    uwb_cb.UwbOperatinMode = state;
}
