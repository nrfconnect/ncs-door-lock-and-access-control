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

#ifdef CONFIG_ALIRO_BLE_TP
constexpr CryptoTypes::KeyId kGroupResolvingKeyId{ kKeyIdRangeBegin + 0x0001 };
#endif // CONFIG_ALIRO_BLE_TP

} // namespace Aliro
