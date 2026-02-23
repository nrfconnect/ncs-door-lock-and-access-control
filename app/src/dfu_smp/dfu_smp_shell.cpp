/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "dfu_smp_manager.h"

#include <aliro/utils.h>

#include <zephyr/shell/shell.h>

namespace {

int ShellCmdHandleDfuSmpOn(const struct shell *shell, size_t argc, char **)
{
	VerifyOrReturnValue(argc == 1, -EINVAL, shell_print(shell, "Usage: dl dfu_smp on"));

	VerifyOrReturnValue(!Aliro::Dfu::SmpManager::Instance().IsSmpEnabled(), -EINVAL,
			    shell_print(shell, "DFU BLE SMP already started"));
	Aliro::Dfu::SmpManager::Instance().Toggle();

	shell_print(shell, "DFU BLE SMP started");
	return 0;
}

int ShellCmdHandleDfuSmpOff(const struct shell *shell, size_t argc, char **)
{
	VerifyOrReturnValue(argc == 1, -EINVAL, shell_print(shell, "Usage: dl dfu_smp off"));

	VerifyOrReturnValue(Aliro::Dfu::SmpManager::Instance().IsSmpEnabled(), -EINVAL,
			    shell_print(shell, "DFU BLE SMP already stopped"));
	Aliro::Dfu::SmpManager::Instance().Toggle();

	shell_print(shell, "DFU BLE SMP stopped");
	return 0;
}

SHELL_STATIC_SUBCMD_SET_CREATE(dfu_smp_cmd, SHELL_CMD(on, NULL, "Enable DFU BLE SMP", ShellCmdHandleDfuSmpOn),
			       SHELL_CMD(off, NULL, "Disable DFU BLE SMP", ShellCmdHandleDfuSmpOff),
			       SHELL_SUBCMD_SET_END);

} // namespace

SHELL_SUBCMD_ADD((dl), dfu_smp, &dfu_smp_cmd, "Enable/disable DFU BLE SMP: dl dfu_smp <on|off>", NULL, 0, 0);
