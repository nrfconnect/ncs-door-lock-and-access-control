/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include <psa/crypto.h>
#include <psa/protected_storage.h>

namespace DoorLock::ReaderStorage {

/** @brief PSA Protected Storage UID for persisted Reader identifier. */
constexpr psa_storage_uid_t kReaderIdentifierUid{ CONFIG_DOOR_LOCK_ALIRO_READER_STORAGE_IDENTIFIER_PSA_STORAGE_UID };

/** @brief PSA key ID for the Reader private key. */
constexpr psa_key_id_t kPrivateKeyId{ CONFIG_DOOR_LOCK_ALIRO_READER_STORAGE_PRIVATE_KEY_PSA_KEY_ID };

#ifdef CONFIG_DOOR_LOCK_ALIRO_READER_STORAGE_GROUP_RESOLVING_KEY_SUPPORT
/** @brief PSA key ID for the Group Resolving Key. */
constexpr psa_key_id_t kGroupResolvingKeyId{ CONFIG_DOOR_LOCK_ALIRO_READER_STORAGE_GROUP_RESOLVING_KEY_PSA_KEY_ID };
#endif // CONFIG_DOOR_LOCK_ALIRO_READER_STORAGE_GROUP_RESOLVING_KEY_SUPPORT

} // namespace DoorLock::ReaderStorage
