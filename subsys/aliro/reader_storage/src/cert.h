/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include <aliro/errors.h>

#include <cstddef>

namespace DoorLock::ReaderStorage {

/**
 * @brief Maximum Reader certificate size accepted by the storage.
 */
constexpr size_t kMaxCertificateSize{ CONFIG_DOOR_LOCK_ALIRO_READER_STORAGE_CERTIFICATE_MAX_SIZE };

/**
 * @brief Loads Reader certificate and System Issuer CA public key data from persistent storage to the in-memory cache.
 *
 * @return ALIRO_NO_ERROR on success, error code otherwise.
 */
AliroError LoadCert();

/**
 * @brief Clears Reader certificate and System Issuer CA public key data from the persistent storage and cache.
 *
 * @return ALIRO_NO_ERROR on success, error code otherwise.
 */
AliroError ClearCert();

} // namespace DoorLock::ReaderStorage
