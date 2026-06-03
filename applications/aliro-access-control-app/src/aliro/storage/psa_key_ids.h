/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

#pragma once

#include <aliro/types.h>

#include <zephyr/sys/util.h>

#include <cstddef>

namespace DoorLock::Storage::PsaKeyIds {

using KeyId = Aliro::CryptoTypes::KeyId;

// Key ID range for Aliro keys
constexpr KeyId kKeyIdRangeBegin{ CONFIG_DOOR_LOCK_ALIRO_PSA_KEY_ID_RANGE_BASE };
constexpr size_t kKeyIdRangeSize{ 0x10000 }; /* 64KB */

static_assert(IN_RANGE(CONFIG_DOOR_LOCK_ALIRO_READER_STORAGE_PRIVATE_KEY_PSA_KEY_ID, kKeyIdRangeBegin,
		       kKeyIdRangeBegin + kKeyIdRangeSize - 1),
	      "Reader Private Key PSA Key ID is out of Aliro key ID range");

#ifdef CONFIG_DOOR_LOCK_BLE_UWB
static_assert(IN_RANGE(CONFIG_DOOR_LOCK_ALIRO_READER_STORAGE_GROUP_RESOLVING_KEY_PSA_KEY_ID, kKeyIdRangeBegin,
		       kKeyIdRangeBegin + kKeyIdRangeSize - 1),
	      "Group Resolving Key PSA Key ID is out of Aliro key ID range");
#endif // CONFIG_DOOR_LOCK_BLE_UWB

#ifdef CONFIG_DOOR_LOCK_CREDENTIAL_ISSUER_CA
constexpr KeyId kCredentialIssuerCAPublicKeyId{ kKeyIdRangeBegin + 0x0002 };
#endif // CONFIG_DOOR_LOCK_CREDENTIAL_ISSUER_CA

#ifdef CONFIG_DOOR_LOCK_EXPEDITED_FAST_PHASE
constexpr KeyId kKpersistentRangeBegin{ CONFIG_DOOR_LOCK_ALIRO_PSA_KEY_ID_RANGE_BASE + 0x01000 };
constexpr size_t kKpersistentRangeSize{ 0x01000 }; /* 4KB */
#endif // CONFIG_DOOR_LOCK_EXPEDITED_FAST_PHASE

} // namespace DoorLock::Storage::PsaKeyIds
