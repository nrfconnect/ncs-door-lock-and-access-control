/*
 * Copyright 2024-2025 NXP.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * \file  phOsalUwb_Thread_Zephyr.c
 * @brief OSAL Implementation.
 */

/** \addtogroup grp_osal_uwb
    @{
 */
/** Header Files */

#ifdef UWBIOT_USE_FTR_FILE
#include "uwb_iot_ftr.h"
#else
#include "uwb_iot_ftr_default.h"
#endif

#include "phOsalUwb.h"
#include "phOsalUwb_Internal.h"
#include "phNxpLogApis_HalUtils.h"

static void phOsalUwb_ThreadEntry(void *entry, void *context, void *unused)
{
    ARG_UNUSED(unused);

    ((pphOsalUwb_ThreadFunction_t)entry)(context);
}

/** Function Definitions */
UWBSTATUS phOsalUwb_Thread_Create(void **hThread, pphOsalUwb_ThreadFunction_t pThreadFunction, void *pParam)
{
    UWBSTATUS wCreateStatus                             = UWBSTATUS_FAILED;
    /*Thread create Parameters handle*/
    phOsalUwb_ThreadCreationParams_t *pThreadParams     = (phOsalUwb_ThreadCreationParams_t *)pParam;
    /*Thread Handle*/
    phOsalUwb_sOsalThreadHandle_t *pOsalThreadHandle    = NULL;

    if (k_is_in_isr())
    {
        wCreateStatus = PHUWBSTVAL(CID_UWB_OSAL, UWBSTATUS_PENDING);
    }
    else if ((NULL == hThread) || (NULL == pThreadFunction) || (NULL == pThreadParams)) {
        wCreateStatus = PHUWBSTVAL(CID_UWB_OSAL, UWBSTATUS_INVALID_PARAMETER);
    }
    else {
        PHUWB_UNUSED(pThreadParams->stackdepth);
        *hThread = NULL;
        /*Allocate memory for Thread Handle*/
        pOsalThreadHandle =(phOsalUwb_sOsalThreadHandle_t *)phOsalUwb_GetMemory(sizeof(phOsalUwb_sOsalThreadHandle_t));
        /* Check whether memory is valid*/
        if (pOsalThreadHandle == NULL) {
            wCreateStatus = PHUWBSTVAL(CID_UWB_OSAL, PH_OSALUWB_THREAD_CREATION_ERROR);
        }
        else{
            /*initialize Thread object Handle*/
            pOsalThreadHandle->zthread_Handle       = NULL;
            /*Allocate memory for Thread Handle*/
            pOsalThreadHandle->zthread_Handle       = (k_thread_t *)phOsalUwb_GetMemory(sizeof(k_thread_t));
            /* Check whether memory is valid*/
            if (pOsalThreadHandle->zthread_Handle == NULL) {
                wCreateStatus = PHUWBSTVAL(CID_UWB_OSAL, PH_OSALUWB_THREAD_CREATION_ERROR);
                phOsalUwb_FreeMemory(pOsalThreadHandle);
            }
            else {
                /* Initialize memory for Thread Stack Handle */

                pOsalThreadHandle->zthread_Stack_Handle = pThreadParams->pStack;
                pOsalThreadHandle->zthread_Stack_Size =  (size_t)pThreadParams->stackdepth;
                /** This routine initializes a thread, then schedules it for execution.
                 *  Parameters as follows
                 *  Pointer to uninitialized struct k_thread
                 *  Pointer to the stack space.
                 *  Stack size in bytes.
                 *  Thread entry function.
                 *  1st entry point parameter.
                 *  2nd entry point parameter.
                 *  3rd entry point parameter.
                 *  Thread priority.
                 *  Thread options.
                 *  Scheduling delay, or K_NO_WAIT for no delay.
                 */
                pOsalThreadHandle->zthread_Tid = k_thread_create(pOsalThreadHandle->zthread_Handle,
                    pOsalThreadHandle->zthread_Stack_Handle,
                    pOsalThreadHandle->zthread_Stack_Size,
                    phOsalUwb_ThreadEntry,
                    (void *)pThreadFunction,
                    pThreadParams->pContext,
                    NULL,
                    K_PRIO_PREEMPT(pThreadParams->priority),
                    0,
                    K_NO_WAIT);

                if (pOsalThreadHandle->zthread_Tid == NULL) {
                    wCreateStatus = PHUWBSTVAL(CID_UWB_OSAL, PH_OSALUWB_THREAD_CREATION_ERROR);
                    /* Clear allocated memory*/
                    phOsalUwb_FreeMemory(pOsalThreadHandle->zthread_Handle);
                    phOsalUwb_FreeMemory(pOsalThreadHandle);
                }
                else {
                    /* Set thread name */
                    k_thread_name_set(pOsalThreadHandle->zthread_Tid,  pThreadParams->taskname);
                    /*return success status*/
                    wCreateStatus = PHUWBSTVAL(CID_UWB_OSAL, UWBSTATUS_SUCCESS);
                    /*Updating the thread Handle*/
                    *(hThread) = (void *)pOsalThreadHandle;
                }
            }
        }
    }

    return wCreateStatus;
}

void phOsalUwb_Thread_Join(UWBOSAL_TASK_HANDLE hThread)
{
    /*TODO: Need to handle the status in future ticket dependencies of RTOS & POSIX API'S*/
    if ((hThread != NULL) && (hThread->zthread_Handle != NULL)) {
        /* Sleep until a thread exits.
         * The caller will be put to sleep until the target thread exits, either
         * due to being aborted, self-exiting, or taking a fatal error. This API
         * returns immediately if the thread isn't running.*/
        (void)k_thread_join(hThread->zthread_Handle, K_FOREVER);
    }

    return;
}

UWBSTATUS phOsalUwb_Thread_Delete(UWBOSAL_TASK_HANDLE hThread)
{
    UWBSTATUS wDeletionStatus = UWBSTATUS_FAILED;

    /*TODO: Need to handle the status in future ticket dependencies of RTOS & POSIX API'S*/
    if ((hThread != NULL) && (hThread->zthread_Tid != NULL)) {
        /*This routine permanently stops execution of thread.*/
        k_thread_abort(hThread->zthread_Tid);

        if ((hThread->zthread_Handle != NULL)) {
            /* Clear the memory allocated for thread*/
            phOsalUwb_FreeMemory(hThread->zthread_Handle);
            phOsalUwb_FreeMemory(hThread);
            /*return success status*/
            wDeletionStatus = PHUWBSTVAL(CID_UWB_OSAL, UWBSTATUS_SUCCESS);
        }
        else {
            wDeletionStatus = PHUWBSTVAL(CID_UWB_OSAL, PH_OSALUWB_THREAD_DELETE_ERROR);
        }
    }
    else{
        wDeletionStatus = PHUWBSTVAL(CID_UWB_OSAL, PH_OSALUWB_THREAD_DELETE_ERROR);
    }

    return wDeletionStatus;
}

void phOsalUwb_TaskResume(UWBOSAL_TASK_HANDLE hThread)
{
    if ((hThread != NULL) && (hThread->zthread_Tid != NULL)) {
        /* Resume a suspended thread.
         * This routine allows the kernel scheduler to make thread the current thread,
         * when it is next eligible for that role.*/
        k_thread_resume(hThread->zthread_Tid);
    }
}

void phOsalUwb_TaskSuspend(UWBOSAL_TASK_HANDLE hThread)
{
    if ((hThread != NULL) && (hThread->zthread_Tid != NULL)) {
    /* Suspend a thread.
     * This routine prevents the kernel scheduler from making thread the current thread.
     * All other internal operations on thread are still performed */
    k_thread_suspend(hThread->zthread_Tid);
    }
}

void phOsalUwb_TaskStartScheduler(void)
{
    /* Yield the current thread.
     * This routine causes the current thread to yield execution to another thread of the
     * same or higher priority. If there are no other ready threads of the same or higher
     * priority, the routine returns immediately.*/
    k_yield();
}

void phOsalUwb_Thread_Context_Switch(void)
{
    /*Check whether it is possible to yield in the current context*/
    if (k_can_yield())
    {
        k_yield();
    }
}

/** @} */
