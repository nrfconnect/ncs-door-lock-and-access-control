/*
 * Local Zephyr queue override for the TSRR250/SR250 UWB stack integration.
 *
 * The upstream Zephyr OSAL queue implementation assumes every queue handle is
 * valid. During nRF54LM20 bring-up, early UWB messages can arrive with a bad
 * handle; validating here keeps the system alive and leaves a useful log.
 */

#include "phOsalUwb.h"
#include "phOsalUwb_Internal.h"
#include "uwb_board.h"

#include <stdint.h>
#include <stdio.h>
#include <zephyr/kernel.h>

#define PHOSALUWB_QUEUE_MAGIC 0x51555742u
#define PHOSALUWB_QUEUE_DEAD_MAGIC 0x44455742u

typedef struct {
    uint32_t magic;
    struct k_msgq msgq;
    uint8_t *buffer;
} phOsalUwb_Queue_t;

static phOsalUwb_Queue_t *queue_from_handle(intptr_t msqid, const char *operation)
{
    uintptr_t addr = (uintptr_t)msqid;

    if (addr == 0U) {
        printf("[TSRR250][OSAL] %s with null queue handle\n", operation);
        return NULL;
    }

    if ((addr & (sizeof(void *) - 1U)) != 0U || addr < 0x20000000U || addr >= 0x40000000U) {
        printf("[TSRR250][OSAL] %s with invalid queue handle 0x%08lx\n",
               operation, (unsigned long)addr);
        return NULL;
    }

    phOsalUwb_Queue_t *pQueue = (phOsalUwb_Queue_t *)addr;

    if (pQueue->magic != PHOSALUWB_QUEUE_MAGIC) {
        printf("[TSRR250][OSAL] %s with non-UWB queue handle 0x%08lx magic 0x%08lx\n",
               operation, (unsigned long)addr, (unsigned long)pQueue->magic);
        return NULL;
    }

    if (pQueue->buffer == NULL) {
        printf("[TSRR250][OSAL] %s with queue 0x%08lx missing buffer\n",
               operation, (unsigned long)addr);
        return NULL;
    }

    return pQueue;
}

intptr_t phOsalUwb_msgget(uint32_t queueLength)
{
    if (queueLength == 0U) {
        return 0;
    }

    phOsalUwb_Queue_t *pQueue = (phOsalUwb_Queue_t *)phOsalUwb_GetMemory(sizeof(phOsalUwb_Queue_t));
    if (pQueue == NULL) {
        return 0;
    }

    pQueue->magic = 0U;
    pQueue->buffer = (uint8_t *)phOsalUwb_GetMemory(queueLength * sizeof(phLibUwb_Message_t));
    if (pQueue->buffer == NULL) {
        phOsalUwb_FreeMemory(pQueue);
        return 0;
    }

    k_msgq_init(&pQueue->msgq, pQueue->buffer, sizeof(phLibUwb_Message_t), queueLength);
    pQueue->magic = PHOSALUWB_QUEUE_MAGIC;

    return (intptr_t)pQueue;
}

void phOsalUwb_msgrelease(intptr_t msqid)
{
    phOsalUwb_Queue_t *pQueue = queue_from_handle(msqid, "msgrelease");

    if (pQueue == NULL) {
        return;
    }

    pQueue->magic = PHOSALUWB_QUEUE_DEAD_MAGIC;

    if (pQueue->buffer != NULL) {
        phOsalUwb_FreeMemory(pQueue->buffer);
        pQueue->buffer = NULL;
    }

    phOsalUwb_FreeMemory(pQueue);
}

UWBSTATUS phOsalUwb_msgsnd(intptr_t msqid, phLibUwb_Message_t *msg, unsigned long waittimeout)
{
    phOsalUwb_Queue_t *pQueue = queue_from_handle(msqid, "msgsnd");

    if (pQueue == NULL || msg == NULL) {
        return UWBSTATUS_FAILED;
    }

    int ret = k_is_in_isr() ? k_msgq_put(&pQueue->msgq, msg, K_NO_WAIT) :
                              k_msgq_put(&pQueue->msgq, msg, K_MSEC(waittimeout));

    return (ret == 0) ? UWBSTATUS_SUCCESS : UWBSTATUS_FAILED;
}

UWBSTATUS phOsalUwb_msgrcv(intptr_t msqid, phLibUwb_Message_t *msg, unsigned long waittimeout)
{
    phOsalUwb_Queue_t *pQueue = queue_from_handle(msqid, "msgrcv");

    if (pQueue == NULL || msg == NULL) {
        return UWBSTATUS_FAILED;
    }

    int ret = k_msgq_get(&pQueue->msgq, msg, K_MSEC(waittimeout));

    return (ret == 0) ? UWBSTATUS_SUCCESS : UWBSTATUS_FAILED;
}

int phOsalUwb_queueSpacesAvailable(intptr_t msqid)
{
    phOsalUwb_Queue_t *pQueue = queue_from_handle(msqid, "queueSpacesAvailable");

    if (pQueue == NULL) {
        return 0;
    }

    return k_msgq_num_free_get(&pQueue->msgq);
}

UWBSTATUS phOsalUwb_msgreset(intptr_t msqid)
{
    phOsalUwb_Queue_t *pQueue = queue_from_handle(msqid, "msgreset");

    if (pQueue == NULL) {
        return UWBSTATUS_FAILED;
    }

    k_msgq_purge(&pQueue->msgq);

    return UWBSTATUS_SUCCESS;
}
