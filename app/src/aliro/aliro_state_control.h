/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
#pragma once

#include <aliro/errors.h>

namespace DoorLock::AliroStateControl {

/**
 * @brief Updates Aliro runtime state: start when provisioned, otherwise stop.
 *
 * @return ALIRO_NO_ERROR on success, error status otherwise.
 */
AliroError UpdateAliroState();

} // namespace DoorLock::AliroStateControl
