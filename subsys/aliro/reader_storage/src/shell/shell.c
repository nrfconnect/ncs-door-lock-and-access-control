/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/shell/shell.h>

SHELL_SUBCMD_SET_CREATE(reader_storage_cmd, (reader));
SHELL_CMD_REGISTER(reader, &reader_storage_cmd, "Reader storage commands", NULL);
