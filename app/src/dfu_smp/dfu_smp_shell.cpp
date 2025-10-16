/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "dfu_smp_shell.h"
#include "dfu_smp_manager.h"
#include "aliro/utils.h"
#include <zephyr/shell/shell.h>

int ShellCmdHandleDfuSmp(const struct shell *shell, size_t argc, char **argv)
{
	using namespace Aliro::Dfu;

	VerifyOrReturnValue(IN_RANGE(argc, 2, 2), -EINVAL, shell_print(shell, "Usage: dl dfu_smp <on|off>"));

	if (strcmp(argv[1], "on") == 0) {
		VerifyOrReturnValue(!SmpManager::Instance().IsSmpEnabled(), -EINVAL,
				    shell_print(shell, "DFU BLE SMP already started"));
		SmpManager::Instance().Toggle();
		shell_print(shell, "DFU BLE SMP started");
		return 0;
	}

	if (strcmp(argv[1], "off") == 0) {
		VerifyOrReturnValue(SmpManager::Instance().IsSmpEnabled(), -EINVAL,
				    shell_print(shell, "DFU BLE SMP already stopped"));
		SmpManager::Instance().Toggle();
		shell_print(shell, "DFU BLE SMP stopped");
		return 0;
	}

	shell_print(shell, "Usage: dl dfu_smp <on|off>");
	return -EINVAL;
}
