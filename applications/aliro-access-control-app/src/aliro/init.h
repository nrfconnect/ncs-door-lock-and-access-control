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

/**
 * @brief Check if Aliro stack is currently running.
 *
 * @return true if Aliro stack is running, false otherwise.
 */
bool IsAliroRunning();
