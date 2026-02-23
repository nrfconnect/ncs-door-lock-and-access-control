/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include <cstddef>
#include <cstring>

#ifdef CONFIG_DOOR_LOCK_EXPEDITED_FAST_PHASE
#include "aliro/kpersistent_manager/kpersistent_manager.h"
#endif // CONFIG_DOOR_LOCK_EXPEDITED_FAST_PHASE

bool IsShellInitialized();

#ifdef CONFIG_DOOR_LOCK_EXPEDITED_FAST_PHASE

Aliro::KpersistentManager *GetShellKpersistentManager();

#endif // CONFIG_DOOR_LOCK_EXPEDITED_FAST_PHASE

template <size_t N> constexpr size_t CStrLen(const char (&)[N])
{
	return N;
}

template <size_t N> bool CmdMatch(const char *cmd, const char (&cmdStr)[N])
{
	return strncmp(cmd, cmdStr, CStrLen(cmdStr)) == 0;
}
