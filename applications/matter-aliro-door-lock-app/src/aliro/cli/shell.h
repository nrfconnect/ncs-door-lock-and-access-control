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
 * @note This function should be called after system and Aliro stack initialization to ensure all components are
 * properly initialized.
 */
void InitShellCommands(Aliro::KpersistentManager *kpersistentManager);
