/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

/**
 * @file storage.h
 * @brief Low-level NVS storage backend for External NVS.
 *
 * Wraps Zephyr's NVS file system API to provide basic read, write, and
 * delete operations on a flash partition used by the External NVS subsystem.
 */

#pragma once

#include <external_nvs/external_nvs.h>

namespace DoorLock::ExternalNvs::Storage {

/**
 * @brief Initialize the NVS storage backend.
 *
 * Opens the specified flash partition, queries its sector layout, and
 * mounts the NVS file system.
 *
 * @param partitionId Flash partition identifier to open and mount.
 *
 * @retval 0 on success.
 * @retval negative errno code on failure.
 */
int Init(uint8_t partitionId);

/**
 * @brief Erase all entries from the NVS partition.
 *
 * Clears the NVS file system and re-mounts it so that it is ready for
 * subsequent operations.
 *
 * @retval 0 on success.
 * @retval negative errno code on failure.
 */
int Clear();

/**
 * @brief Write raw data to the NVS partition.
 *
 * @param id   NVS entry identifier.
 * @param data Pointer to the data to store.
 * @param len  Length of @p data in bytes.
 *
 * @retval 0 on success.
 * @retval negative errno code on failure.
 */
int Write(Id id, const void *data, size_t len);

/**
 * @brief Read raw data from the NVS partition.
 *
 * @param[in]     id   NVS entry identifier.
 * @param[out]    data Pointer to the buffer that receives the stored data.
 * @param[in,out] len  On input, the capacity of @p data in bytes.
 *                     On output, the number of bytes actually read.
 *
 * @retval 0 on success.
 * @retval negative errno code on failure.
 */
int Read(Id id, void *data, size_t &len);

/**
 * @brief Delete an entry from the NVS partition.
 *
 * @param id NVS entry identifier to delete.
 *
 * @retval 0 on success.
 * @retval negative errno code on failure.
 */
int Delete(Id id);

} // namespace DoorLock::ExternalNvs::Storage
