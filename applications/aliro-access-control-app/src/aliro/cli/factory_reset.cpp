/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "shell_private.h"

#include <aliro/utils.h>
#include <zephyr/shell/shell.h>

#ifdef CONFIG_SETTINGS_NVS
#include <zephyr/fs/nvs.h>
#else // CONFIG_SETTINGS_ZMS
#include <zephyr/fs/zms.h>
#endif // CONFIG_SETTINGS_NVS || CONFIG_SETTINGS_ZMS

#include <zephyr/settings/settings.h>
#include <zephyr/sys/reboot.h>

namespace {

K_WORK_DELAYABLE_DEFINE(sRebootWork, [](k_work *) { sys_reboot(SYS_REBOOT_WARM); });

int ShellCmdHandleFactoryReset(const struct shell *shell, size_t, char **)
{
	VerifyOrReturnValue(IsShellInitialized(), -EIO, shell_warn(shell, "Not initialized yet\n"));

	void *storage{ nullptr };
	int status = settings_storage_get(&storage);
	VerifyOrReturnStatus(status == 0, -EIO, shell_warn(shell, "Cannot get storage\n"));

#ifdef CONFIG_SETTINGS_NVS
	status = nvs_clear(static_cast<nvs_fs *>(storage));
#else // CONFIG_SETTINGS_ZMS
	status = zms_clear(static_cast<zms_fs *>(storage));
#endif // CONFIG_SETTINGS_NVS || CONFIG_SETTINGS_ZMS

	VerifyOrReturnStatus(status == 0, -EIO, shell_warn(shell, "Cannot clear storage\n"));
	k_work_reschedule(&sRebootWork, K_MSEC(250));
	return 0;
}

} // namespace

SHELL_SUBCMD_ADD((dl), factory_reset, NULL, "Factory reset", ShellCmdHandleFactoryReset, 0, 0);
