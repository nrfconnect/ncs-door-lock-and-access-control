/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include <cstdint>

namespace DoorLock::AliroService {

/**
 * @brief Tracks the Aliro service lifecycle state.
 */
class State {
public:
	/**
	 * @brief Check whether the service has been initialized.
	 *
	 * @retval true The service is initialized or started.
	 * @retval false The service is uninitialized.
	 */
	bool IsInitialized() const { return mValue != Value::Uninitialized; }

	/**
	 * @brief Check whether the service has been started.
	 *
	 * @retval true The service is started.
	 * @retval false The service is not started.
	 */
	bool IsStarted() const { return mValue == Value::Started; }

	/**
	 * @brief Set the service state to initialized.
	 */
	void SetInitialized() { mValue = Value::Initialized; }

	/**
	 * @brief Set the service state to started.
	 */
	void SetStarted() { mValue = Value::Started; }

	/**
	 * @brief Set the service state to stopped.
	 *
	 * A stopped service remains initialized.
	 */
	void SetStopped() { mValue = Value::Initialized; }

private:
	enum class Value : uint8_t { Uninitialized, Initialized, Started };

	Value mValue{ Value::Uninitialized };
};

} // namespace DoorLock::AliroService
