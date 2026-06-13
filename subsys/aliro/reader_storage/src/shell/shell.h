/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include <reader_storage/reader.h>

namespace DoorLock::ReaderStorage {

#ifdef CONFIG_DOOR_LOCK_ALIRO_READER_STORAGE_SHELL

/**
 * @brief Indicates whether Reader storage was initialized.
 *
 * Used by shell commands to guard operations until `ReaderStorage::Init()` completes.
 *
 * @return True when initialized, false otherwise.
 */
bool IsInitialized();

/**
 * @brief Notifies listener that shell command changed Reader data.
 *
 * Triggers the callback registered through `ReaderStorage::Init()`.
 */
void NotifyDataChanged();

#endif // CONFIG_DOOR_LOCK_ALIRO_READER_STORAGE_SHELL

} // namespace DoorLock::ReaderStorage
