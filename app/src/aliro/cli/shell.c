/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/shell/shell.h>

SHELL_SUBCMD_SET_CREATE(door_lock_cmd, (dl));
SHELL_CMD_REGISTER(dl, &door_lock_cmd, "Door lock commands", NULL);
