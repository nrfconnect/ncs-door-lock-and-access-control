/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <aliro/aliro.h>

#include <zephyr/shell/shell.h>

namespace {

const char *GetReaderChipName(void)
{
#if defined(CONFIG_ST25R200_DRV)
	return "ST25R100";
#elif defined(CONFIG_ST25R500_DRV)
	return "ST25R300";
#else
	return "Unknown NFC reader driver";
#endif
}

int ShellCmdHandleInfo(const struct shell *shell, size_t, char **)
{
	shell_print(shell, "Aliro version: %s", Aliro::AliroStack::GetLibraryVersion());
	shell_print(shell, "NFC reader: %s", GetReaderChipName());
	return 0;
}

} // namespace

SHELL_SUBCMD_ADD((dl), info, NULL, "Show Aliro lib version and NFC reader chip name", ShellCmdHandleInfo, 0, 0);
