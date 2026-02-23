/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

#pragma once

#include <zephyr/kernel.h>

#include <tuple>

/**
 * @brief RAII-style mutex guard.
 *
 * Locks a mutex upon creation and unlocks it upon destruction.
 */
struct MutexGuard {
	/**
	 * @brief Constructor for MutexGuard.
	 * Locks the mutex when the MutexGuard object is created.
	 *
	 * @param mutex Pointer to the mutex to lock.
	 */
	explicit MutexGuard(k_mutex &mutex) noexcept : mMutex{ mutex }
	{
		std::ignore = k_mutex_lock(&mMutex, K_FOREVER);
	}

	/**
	 * @brief Destructor for MutexGuard.
	 *
	 * Unlocks the mutex when the MutexGuard object is destroyed.
	 */
	~MutexGuard() noexcept { k_mutex_unlock(&mMutex); }

	// Disable copy and move constructors and assignment operators.
	MutexGuard(const MutexGuard &) = delete;
	MutexGuard(MutexGuard &&) = delete;
	MutexGuard &operator=(const MutexGuard &) = delete;
	MutexGuard &operator=(MutexGuard &&) = delete;

private:
	k_mutex &mMutex;
};
