/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "shell_private.h"

#include "aliro/kpersistent_manager/kpersistent_manager.h"

namespace {

bool sShellInitialized{ false };

#ifdef CONFIG_DOOR_LOCK_EXPEDITED_FAST_PHASE
Aliro::KpersistentManager *sKpersistentManager{ nullptr };
#endif // CONFIG_DOOR_LOCK_EXPEDITED_FAST_PHASE

} // namespace

bool IsShellInitialized()
{
	return sShellInitialized;
}

#ifdef CONFIG_DOOR_LOCK_EXPEDITED_FAST_PHASE

Aliro::KpersistentManager *GetShellKpersistentManager()
{
	return sKpersistentManager;
}

#endif // CONFIG_DOOR_LOCK_EXPEDITED_FAST_PHASE

void InitShellCommands([[maybe_unused]] Aliro::KpersistentManager *kpersistentManager)
{
#ifdef CONFIG_DOOR_LOCK_EXPEDITED_FAST_PHASE
	sKpersistentManager = kpersistentManager;
#endif // CONFIG_DOOR_LOCK_EXPEDITED_FAST_PHASE

	sShellInitialized = true;
}
