/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <aliro/interface.h>

#include <doorlock/utils/utils.h>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/atomic.h>

LOG_MODULE_REGISTER(interface_os_mutex, CONFIG_DOOR_LOCK_ALIRO_INTERFACE_IMPL_OS_LOG_LEVEL);

namespace Aliro::Interface::Os::Mutex {

namespace {

K_MUTEX_DEFINE(sStackSessionMutex);

} // namespace

void Lock()
{
	const int lockErr = k_mutex_lock(&sStackSessionMutex, K_FOREVER);
	VerifyOrDie(lockErr == 0, "Stack session mutex lock failed");
}

void Unlock()
{
	const int unlockErr = k_mutex_unlock(&sStackSessionMutex);
	VerifyOrDie(unlockErr == 0, "Stack session mutex unlock failed");
}

} // namespace Aliro::Interface::Os::Mutex
