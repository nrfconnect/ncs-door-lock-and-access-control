/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

/**
 * @brief Registers shell commands for managing a door lock system.
 *
 * This function sets up the Zephyr shell environment with specific commands related to the door lock.
 * It integrates several subcommands under the main command 'dl'. Each subcommand is designed to handle
 * different aspects of the door lock configuration and control.
 *
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
 * @note This function should be called during the system initialization phase to ensure all commands are properly
 * registered before the shell is used.
 */
void RegisterShellCommands();
