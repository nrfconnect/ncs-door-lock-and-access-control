/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include "aliro/kpersistent_manager/kpersistent_manager.h"

/**
 * @brief Initializes shell commands for managing a door lock system.
 *
 * Following commands are available:
 * ## Subcommands
 * - `install`:
 *   - `identifier [value]`: Gets or sets the full reader identifier (concatenation of group identifier and group
 * sub-identifier). If a value is provided, it sets the reader identifier to that value.
 *   - `group_id [value]`: Gets or sets the group identifier (the first 16 bytes of the reader identifier). If a value
 * is provided, it sets the group ID to that value.
 *   - `group_sub_id [value]`: Gets or sets the group sub-identifier (the last 16 bytes of the reader identifier). If a
 * value is provided, it sets the group sub ID to that value.
 * - `provisioning`:
 *   - `AC_key [value]`: Gets or sets the Access Credential public key. If a value is provided, it sets the public key
 * to that value.
 * - `factory_reset`: Executes a factory reset of the system, clearing all stored settings and data.
 *
 * ## Usage
 * Commands are used through the shell interface provided by the Zephyr OS. Each command might require specific
 * parameters as detailed above. The commands are accessed by prefixing them with 'dl', e.g., `dl install group_id`.
 *
 * @note This function should be called after system and Aliro stack initialization to ensure all components are
 * properly initialized.
 */
void InitShellCommands(Aliro::KpersistentManager *kpersistentManager);
