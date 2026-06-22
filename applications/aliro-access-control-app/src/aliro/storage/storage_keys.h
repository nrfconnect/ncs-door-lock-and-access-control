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

constexpr char kStorageKeyNameAccessCredentialPublicKey[] = "AC_key";
constexpr char kStorageKeyNameCredentialIssuerPublicKey[] = "CI_key";

#ifdef CONFIG_DOOR_LOCK_STEP_UP_PHASE

constexpr char kStorageKeyNameCredentialIssuerValidityIteration[] = "VI";

#endif // CONFIG_DOOR_LOCK_STEP_UP_PHASE

} // namespace Aliro::StorageKeys
