/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include <aliro/errors.h>

namespace DoorLock::ReaderStorage {

/**
 * @brief Clears Group Resolving Key data from persistent storage.
 *
 * @return ALIRO_NO_ERROR on success, error code otherwise.
 */
AliroError ClearGroup();

} // namespace DoorLock::ReaderStorage
