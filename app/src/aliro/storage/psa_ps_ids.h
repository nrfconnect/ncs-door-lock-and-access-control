/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include <psa/protected_storage.h>

namespace DoorLock::Storage::PsaPsIds {

constexpr psa_storage_uid_t kStorageUidBase{ CONFIG_DOOR_LOCK_ALIRO_PSA_PROTECTED_STORAGE_UID_BASE };

constexpr psa_storage_uid_t kReaderIdentifierUid{ kStorageUidBase + 0x0000 };

} // namespace DoorLock::Storage::PsaPsIds
