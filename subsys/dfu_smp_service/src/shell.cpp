/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <dfu_smp_service/dfu_smp_service.h>

#include <doorlock/utils/utils.h>

#include <zephyr/shell/shell.h>

namespace {

namespace DfuSmpService = DoorLock::DfuSmpService;

int ShellCmdHandleDfuSmpOn(const struct shell *shell, size_t argc, char **)
{
	VerifyOrReturnValue(argc == 1, -EINVAL, shell_print(shell, "Usage: dfu_smp on"));

	VerifyOrReturnValue(!DfuSmpService::IsSmpEnabled(), -EINVAL, shell_print(shell, "DFU BLE SMP already started"));
	DfuSmpService::Toggle();

	shell_print(shell, "DFU BLE SMP started");
	return 0;
}

int ShellCmdHandleDfuSmpOff(const struct shell *shell, size_t argc, char **)
{
	VerifyOrReturnValue(argc == 1, -EINVAL, shell_print(shell, "Usage: dfu_smp off"));

	VerifyOrReturnValue(DfuSmpService::IsSmpEnabled(), -EINVAL, shell_print(shell, "DFU BLE SMP already stopped"));
	DfuSmpService::Toggle();

	shell_print(shell, "DFU BLE SMP stopped");
	return 0;
}

SHELL_STATIC_SUBCMD_SET_CREATE(dfu_smp_cmd, SHELL_CMD(on, NULL, "Enable DFU BLE SMP", ShellCmdHandleDfuSmpOn),
			       SHELL_CMD(off, NULL, "Disable DFU BLE SMP", ShellCmdHandleDfuSmpOff),
			       SHELL_SUBCMD_SET_END);

SHELL_CMD_REGISTER(dfu_smp, &dfu_smp_cmd, "DFU BLE SMP commands", NULL);

} // namespace
