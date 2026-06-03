/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include <psa/protected_storage.h>
#include <zephyr/sys/util.h>

#include <cstdint>

namespace DoorLock::Storage::PsaPsIds {

constexpr psa_storage_uid_t kStorageUidBase{ CONFIG_DOOR_LOCK_ALIRO_PSA_PROTECTED_STORAGE_UID_BASE };
constexpr uint64_t kStorageUidSize{ 0x100000000 }; /* 4GB */

static_assert(IN_RANGE(CONFIG_DOOR_LOCK_ALIRO_READER_STORAGE_IDENTIFIER_PSA_STORAGE_UID, kStorageUidBase,
		       kStorageUidBase + kStorageUidSize - 1),
	      "Reader Identifier PSA Storage UID is out of Aliro PSA Protected Storage UID range");

} // namespace DoorLock::Storage::PsaPsIds
