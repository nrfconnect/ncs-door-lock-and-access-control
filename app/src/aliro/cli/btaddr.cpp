/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/bluetooth/bluetooth.h>

#include <zephyr/shell/shell.h>

namespace {

int ShellCmdHandleBtAddr(const struct shell *shell, size_t, char **)
{
	bt_addr_le_t address[1];
	size_t count{ ARRAY_SIZE(address) };
	bt_id_get(address, &count);

	char addr_str[BT_ADDR_LE_STR_LEN];
	bt_addr_le_to_str(address, addr_str, sizeof(addr_str));

	shell_print(shell, "%s", addr_str);
	return 0;
}

} // namespace

SHELL_SUBCMD_ADD((dl), btaddr, NULL, "Show BLE address", ShellCmdHandleBtAddr, 0, 0);
