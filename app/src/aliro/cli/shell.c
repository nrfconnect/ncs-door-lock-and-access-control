/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/shell/shell.h>

#ifdef CONFIG_DOOR_LOCK_CLI_PROVISIONING
SHELL_SUBCMD_SET_CREATE(provisioning_cmd, (provisioning));
SHELL_SUBCMD_ADD((dl), provisioning, &provisioning_cmd, "Provisioning commands", NULL, 0, 0);
#endif // CONFIG_DOOR_LOCK_CLI_PROVISIONING

#ifdef CONFIG_DOOR_LOCK_CLI_READER
SHELL_SUBCMD_SET_CREATE(reader_cmd, (reader));
SHELL_SUBCMD_ADD((dl), reader, &reader_cmd, "Reader data commands", NULL, 0, 0);
#endif // CONFIG_DOOR_LOCK_CLI_READER

SHELL_SUBCMD_SET_CREATE(door_lock_cmd, (dl));
SHELL_CMD_REGISTER(dl, &door_lock_cmd, "Door lock commands", NULL);
