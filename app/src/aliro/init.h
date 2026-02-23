/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
#pragma once

#include <aliro/errors.h>

/**
 * @brief Initializes the Aliro stack.
 *
 * @return EXIT_SUCCESS if the Aliro stack was initialized successfully, EXIT_FAILURE otherwise.
 */
int AliroInit();

/**
 * @brief Starts the Aliro stack.
 *
 * @return EXIT_SUCCESS if the Aliro stack was started successfully, EXIT_FAILURE otherwise.
 */
int AliroStart();

/**
 * @brief Stops the Aliro stack.
 *
 * @return EXIT_SUCCESS if the Aliro stack was stopped successfully, EXIT_FAILURE otherwise.
 */
int AliroStop();

#ifdef CONFIG_DOOR_LOCK_BLE_UWB

/**
 * @brief Starts or refreshes Aliro BLE advertising with current reader identity data.
 *
 * @return ALIRO_NO_ERROR on success, error status otherwise.
 */
AliroError StartAliroAdvertising();

#endif // CONFIG_DOOR_LOCK_BLE_UWB

/**
 * @brief Check if Aliro stack is currently running.
 *
 * @return true if Aliro stack is running, false otherwise.
 */
bool IsAliroRunning();

#ifdef CONFIG_CHIP

/**
 * @brief Clears Aliro storage.
 *
 * @param reinitializeStorage Whether to reinitialize the storage after clearing.
 */
void ClearStorageAliro(bool reinitializeStorage);

#endif // CONFIG_CHIP
