/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

/**
 * @file counter.h
 * @brief Monotonic counter and unique ID management for rollback protection.
 *
 * Maintains per-entry monotonic counters stored in the Zephyr settings
 * subsystem (internal flash) to protect External NVS data against replay
 * and rollback attacks. A device-unique random identifier is persisted
 * alongside the counters to bind them to a specific device instance.
 *
 * @note This module is only compiled when
 *       @c CONFIG_DOOR_LOCK_EXTERNAL_NVS_ROLLBACK_PROTECTION is enabled.
 */

#pragma once

#include <external_nvs/external_nvs.h>

#include <array>

namespace DoorLock::ExternalNvs::Counter {

/** @brief Size of the device-unique identifier in bytes. */
constexpr size_t kUniqueIdSize{ CONFIG_DOOR_LOCK_EXTERNAL_NVS_ROLLBACK_PROTECTION_UNIQUE_ID_SIZE };

/** @brief Type representing the device-unique identifier. */
using UniqueId = std::array<uint8_t, kUniqueIdSize>;

/** @brief Type representing a monotonic counter value. */
using Counter = uint32_t;

/**
 * @brief Initialize the counter subsystem.
 *
 * Initializes the Zephyr settings subsystem and attempts to load the
 * persisted unique ID. If no unique ID exists, a new one is generated
 * using PSA Crypto and saved to settings.
 *
 * @retval 0 on success.
 * @retval negative errno code on failure.
 */
int Init();

/**
 * @brief Clear all persisted counters and regenerate the unique ID.
 *
 * Deletes every key under the rollback-protection settings subtree and
 * generates a fresh unique ID. After this call, existing encrypted
 * entries cannot be validated and should be considered invalidated.
 *
 * @retval 0 on success.
 * @retval negative errno code on failure.
 */
int Clear();

/**
 * @brief Retrieve the current device-unique identifier.
 *
 * @param[out] uniqueId Buffer that receives the unique identifier.
 *
 * @retval 0 on success.
 * @retval -ENODEV if the counter subsystem has not been initialized.
 */
int GetUniqueId(UniqueId &uniqueId);

/**
 * @brief Read the current counter value for a given NVS entry.
 *
 * Loads the counter from settings. If no counter exists for @p id the
 * returned value is 0.
 *
 * @param      id      NVS entry identifier.
 * @param[out] counter Buffer that receives the counter value.
 *
 * @retval 0 on success.
 * @retval negative errno code on failure.
 */
int Read(Id id, Counter &counter);

/**
 * @brief Persist a counter value for a given NVS entry.
 *
 * @param id      NVS entry identifier.
 * @param counter Counter value to store.
 *
 * @retval 0 on success.
 * @retval negative errno code on failure.
 */
int Write(Id id, const Counter &counter);

} // namespace DoorLock::ExternalNvs::Counter
