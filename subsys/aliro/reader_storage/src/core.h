/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include <aliro/errors.h>

namespace DoorLock::ReaderStorage {

/**
 * @brief Loads core Reader data from persistent storage to the in-memory cache.
 *
 * @return ALIRO_NO_ERROR on success, error code otherwise.
 */
AliroError LoadCore();

/**
 * @brief Clears core Reader data from persistent storage and cache.
 *
 * @return ALIRO_NO_ERROR on success, error code otherwise.
 */
AliroError ClearCore();

} // namespace DoorLock::ReaderStorage
