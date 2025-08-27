/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
#pragma once

/**
 * @brief Initializes the Aliro stack.
 *
 * @return EXIT_SUCCESS if the Aliro stack was initialized successfully, EXIT_FAILURE otherwise.
 */
int InitAliro();

/**
 * @brief Starts the Aliro stack.
 *
 * @return EXIT_SUCCESS if the Aliro stack was started successfully, EXIT_FAILURE otherwise.
 */
int StartAliro();

/**
 * @brief Stops the Aliro stack.
 *
 * @return EXIT_SUCCESS if the Aliro stack was stopped successfully, EXIT_FAILURE otherwise.
 */
int StopAliro();
