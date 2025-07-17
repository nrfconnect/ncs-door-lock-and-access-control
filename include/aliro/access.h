/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

#pragma once

#include "aliro/errors.h"

namespace Aliro::Access {

/**
 * @brief Access status.
 */
enum Status : uint8_t { Denied, Granted };

/**
 * @brief Callbacks for the command handlers.
 */
struct Callbacks {
	/**
	 * @brief Callback for access attempts.
	 *
	 * This callback is called when an access attempt is made.
	 *
	 * @param status The status of the access attempt.
	 */
	void (*mOnAccessAttempt)(Status status){ nullptr };

	/**
	 * @brief Callback for errors.
	 *
	 * This callback is called when an error occurs.
	 *
	 * @param error The error that occurred.
	 */
	void (*mOnError)(AliroError error){ nullptr };
};

} // namespace Aliro::Access
