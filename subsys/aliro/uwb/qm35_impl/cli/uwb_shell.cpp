/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "uwb_impl.h"

#include <zephyr/shell/shell.h>

namespace {

using namespace Aliro::Uwb;

int ShellCmdQm35Version(const struct shell *shell, size_t, char **)
{
	if (!UltraWideBandInstance().IsInitialized()) {
		shell_warn(shell, "UWB not initialized");
		return 0;
	}

	const char *version = UltraWideBandInstance().GetFirmwareVersion();

	if (version) {
		shell_print(shell, "%s", version);
	} else {
		shell_warn(shell, "QM35 FW version not available (UWB not initialized?)");
	}

	return 0;
}

} // namespace

SHELL_STATIC_SUBCMD_SET_CREATE(uwb_cmd,
			       SHELL_CMD(qm35_fw_version, NULL, "Print QM35 firmware version", ShellCmdQm35Version),
			       SHELL_SUBCMD_SET_END);

SHELL_CMD_REGISTER(uwb, &uwb_cmd, "UWB commands", NULL);
