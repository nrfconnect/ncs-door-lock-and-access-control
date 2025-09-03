/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include <array>
#include <cstddef>

namespace Aliro::StorageKeys {

constexpr size_t kKeyNameMaxLength{ 16 };

using KeyNameBuffer = std::array<char, Aliro::StorageKeys::kKeyNameMaxLength>;

constexpr char kDoorLockBaseKey[] = "dl";
constexpr char kStorageKeyNameIdentifier[] = "identifier";

#ifndef CONFIG_CHIP

constexpr char kStorageKeyNameAccessCredentialPublicKey[] = "AC_key";

#endif // CONFIG_CHIP

} // namespace Aliro::StorageKeys
