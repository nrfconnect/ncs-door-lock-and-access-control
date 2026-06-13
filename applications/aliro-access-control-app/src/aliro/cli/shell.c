/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/shell/shell.h>

SHELL_SUBCMD_SET_CREATE(provisioning_cmd, (provisioning));
SHELL_SUBCMD_ADD((dl), provisioning, &provisioning_cmd, "Provisioning commands", NULL, 0, 0);

SHELL_SUBCMD_SET_CREATE(door_lock_cmd, (dl));
SHELL_CMD_REGISTER(dl, &door_lock_cmd, "Door lock commands", NULL);
