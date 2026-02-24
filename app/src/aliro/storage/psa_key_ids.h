/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

#pragma once

#include "aliro/types.h"

#include <cstddef>

namespace DoorLock::Storage::PsaKeyIds {

using KeyId = Aliro::CryptoTypes::KeyId;

// Key ID range for Aliro keys
constexpr KeyId kKeyIdRangeBegin{ CONFIG_DOOR_LOCK_ALIRO_PSA_KEY_ID_RANGE_BASE };
constexpr size_t kKeyIdRangeSize{ 0x10000 }; /* 64KB */

constexpr KeyId kPrivateKeyId{ kKeyIdRangeBegin + 0x0000 };

#ifdef CONFIG_DOOR_LOCK_BLE_UWB
constexpr KeyId kGroupResolvingKeyId{ kKeyIdRangeBegin + 0x0001 };
#endif // CONFIG_DOOR_LOCK_BLE_UWB

#ifdef CONFIG_DOOR_LOCK_CREDENTIAL_ISSUER_CA
constexpr KeyId kCredentialIssuerCAPublicKeyId{ kKeyIdRangeBegin + 0x0002 };
#endif // CONFIG_DOOR_LOCK_CREDENTIAL_ISSUER_CA

#ifdef CONFIG_DOOR_LOCK_EXPEDITED_FAST_PHASE
constexpr KeyId kKpersistentRangeBegin{ CONFIG_DOOR_LOCK_ALIRO_PSA_KEY_ID_RANGE_BASE + 0x01000 };
constexpr size_t kKpersistentRangeSize{ 0x01000 }; /* 4KB */
#endif // CONFIG_DOOR_LOCK_EXPEDITED_FAST_PHASE

} // namespace DoorLock::Storage::PsaKeyIds
