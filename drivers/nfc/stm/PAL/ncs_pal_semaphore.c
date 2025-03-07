#include "ncs_pal_semaphore.h"

K_SEM_DEFINE(irq_sem, 0, 1);

void ncs_pal_take_semaphore(k_timeout_t timeout_ms)
{
	k_sem_take(&irq_sem, timeout_ms);
}

void ncs_pal_give_semaphore(void)
{
	k_sem_give(&irq_sem);
}
