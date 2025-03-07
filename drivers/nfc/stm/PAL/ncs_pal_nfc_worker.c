#include "ncs_pal_nfc_worker.h"

#include <zephyr/kernel.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(pal_nfc_stack, CONFIG_NFC_LOG_LEVEL);

K_THREAD_STACK_DEFINE(pal_nfc_stack, CONFIG_RFAL_WORKER_THREAD_STACK_SIZE);
struct k_thread pal_nfc_thread;

k_tid_t ncs_pal_nfc_worker_start(thread_func_t thread_func)
{
	return k_thread_create(&pal_nfc_thread, pal_nfc_stack, CONFIG_RFAL_WORKER_THREAD_STACK_SIZE, thread_func, NULL,
			       NULL, NULL, K_PRIO_PREEMPT(CONFIG_RFAL_WORKER_THREAD_PRIORITY), 0, K_NO_WAIT);
}
