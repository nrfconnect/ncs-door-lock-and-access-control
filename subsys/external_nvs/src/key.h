/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

/**
 * @file key.h
 * @brief Key derivation for External NVS AEAD operations.
 *
 * Derives per-entry 256-bit encryption keys from the device's Hardware
 * Unique Key (HUK) using the NVS entry identifier as context, ensuring
 * each stored object is encrypted with a distinct key.
 */

#pragma once

#include <external_nvs/external_nvs.h>

#include <array>

namespace DoorLock::ExternalNvs::Key {

/** @brief Size of the derived key in bytes (256 bits). */
constexpr size_t kKeySize{ 32 };

/** @brief Type representing a 256-bit encryption key. */
using Key = std::array<uint8_t, kKeySize>;

/**
 * @brief Initialize the key derivation subsystem.
 *
 * Verifies that the Hardware Unique Keys have been provisioned on the
 * device. This must succeed before @ref Generate can be used.
 *
 * @retval 0 on success.
 * @retval -ENODEV if the Hardware Unique Keys are not written.
 */
int Init();

/**
 * @brief Derive an encryption key for a given NVS entry.
 *
 * Uses the Hardware Unique Key to derive a 256-bit key that is unique
 * to the specified @p id.
 *
 * @param      id  NVS entry identifier used as derivation context.
 * @param[out] key Buffer that receives the derived key.
 *
 * @retval 0 on success.
 * @retval -EIO if the key derivation operation fails.
 */
int Derive(Id id, Key &key);

} // namespace DoorLock::ExternalNvs::Key
