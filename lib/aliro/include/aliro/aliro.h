/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include "access.h"

namespace Aliro {

/**
 * @brief Aliro stack.
 */
class AliroStack {
public:
	/**
	 * @brief Gets the instance of the Aliro stack.
	 *
	 * @return The instance of the Aliro stack.
	 */
	static AliroStack &Instance()
	{
		static AliroStack sInstance;
		return sInstance;
	}

	/**
	 * @brief Initializes the Aliro stack.
	 *
	 * @param callbacks The user access callbacks.
	 *
	 * @return ALIRO_NO_ERROR if the stack was initialized successfully, an error code otherwise.
	 */
	AliroError Init(Access::Callbacks callbacks);

	/**
	 * @brief Starts the Aliro stack.
	 *
	 * @return ALIRO_NO_ERROR if the stack was started successfully, an error code otherwise.
	 */
	AliroError Start() const;

private:
	Access::Callbacks mCallbacks;
};

} // namespace Aliro
