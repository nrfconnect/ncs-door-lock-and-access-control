/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

/**
 * @file external_nvs.h
 * @brief Public API for the External NVS subsystem.
 *
 * Provides encrypted non-volatile storage on an external flash device.
 * Data is authenticated and encrypted using AEAD (ChaCha20-Poly1305) with
 * hardware-derived keys. Optional rollback protection is available via
 * monotonic counters.
 */

#pragma once

#include <cstddef>
#include <cstdint>

namespace DoorLock::ExternalNvs {

/** @brief Type used to identify entries stored in external NVS. */
using Id = uint16_t;

#ifdef CONFIG_DOOR_LOCK_EXTERNAL_NVS_ROLLBACK_PROTECTION

/** @brief Reserved ID for the Unique ID. */
constexpr Id kUniqueIdReservedId{ 0xFFFF };

#endif // CONFIG_DOOR_LOCK_EXTERNAL_NVS_ROLLBACK_PROTECTION

/**
 * @brief Initialize the External NVS subsystem.
 *
 * Sets up the underlying NVS storage partition, the nonce generator, and
 * (if enabled) the rollback-protection counter.
 *
 * @param partitionId Flash partition identifier to use for NVS storage.
 *
 * @retval 0 on success.
 * @retval negative errno code on failure.
 */
int Init(uint8_t partitionId);

/**
 * @brief Erase all data from the external NVS storage.
 *
 * Clears the rollback-protection counters and wipes the underlying NVS
 * partition. After calling this function, @ref Init must be called again
 * before using the storage APIs.
 *
 * @retval 0 on success.
 * @retval negative errno code on failure.
 */
int Clear();

/**
 * @brief Encrypt and write data to external NVS.
 *
 * The plaintext is encrypted with AEAD using a hardware-derived key and a
 * fresh nonce. Metadata (nonce, and optionally the rollback counter) is
 * stored alongside the ciphertext. On failure the partially written entry is
 * deleted.
 *
 * @param id   Identifier for the NVS entry.
 * @param data Pointer to the plaintext data to write. Must not be @c nullptr.
 * @param len  Length of the plaintext data in bytes. Must not exceed
 *             @c CONFIG_DOOR_LOCK_EXTERNAL_NVS_MAX_DATA_SIZE.
 *
 * @retval 0 on success.
 * @retval -EINVAL if @p data is @c nullptr or @p len exceeds the maximum.
 * @retval negative errno code on other failures.
 */
int Write(Id id, const void *data, size_t len);

/**
 * @brief Read and decrypt data from external NVS.
 *
 * Reads the stored object, verifies its integrity and authenticity via AEAD
 * decryption, and (if enabled) validates the rollback counter. On integrity
 * failure the corrupted entry is deleted from storage.
 *
 * @param[in]     id   Identifier for the NVS entry.
 * @param[out]    data Pointer to the buffer that receives the decrypted data.
 *                     Must not be @c nullptr.
 * @param[in,out] len  On input, the capacity of @p data in bytes.
 *                     On output, the actual length of the decrypted data.
 *
 * @retval 0 on success.
 * @retval -EINVAL if @p data is @c nullptr.
 * @retval negative errno code on other failures (e.g., integrity check).
 */
int Read(Id id, void *data, size_t &len);

/**
 * @brief Delete an entry from external NVS.
 *
 * If rollback protection is enabled the associated counter is incremented
 * before the entry is removed, preventing replay of a stale copy.
 *
 * @param id Identifier for the NVS entry to delete.
 *
 * @retval 0 on success.
 * @retval negative errno code on failure.
 */
int Delete(Id id);

} // namespace DoorLock::ExternalNvs
