/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

#pragma once

#include "aliro/types.h"

namespace Aliro {

// Key ID range for Aliro keys
constexpr CryptoTypes::KeyId kKeyIdRangeBegin{ 0x40000 };
constexpr CryptoTypes::KeyId kKeyIdRangeSize{ 0x10000 };
constexpr CryptoTypes::KeyId kPrivateKeyId{ kKeyIdRangeBegin + 0x0000 };

#ifdef CONFIG_ALIRO_BLE_UWB
constexpr CryptoTypes::KeyId kGroupResolvingKeyId{ kKeyIdRangeBegin + 0x0001 };
#endif // CONFIG_ALIRO_BLE_UWB

#ifdef CONFIG_DOOR_LOCK_EXPEDITED_FAST_PHASE
constexpr CryptoTypes::KeyId kKpersistentRangeBegin{ 0x41000 };
constexpr CryptoTypes::KeyId kKpersistentRangeEnd{ 0x41000 + CONFIG_MAX_NUMBER_OF_KPERSISTENT - 1 };
#endif // CONFIG_DOOR_LOCK_EXPEDITED_FAST_PHASE

} // namespace Aliro
