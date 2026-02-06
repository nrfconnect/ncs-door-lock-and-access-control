/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include "aliro/errors.h"
#include "aliro/platform/ble/ble_advertising_arbiter.h"

#include <bluetooth/services/dfu_smp.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>

#include <array>

namespace Aliro::Dfu {

class SmpManager {
public:
	static SmpManager &Instance()
	{
		static SmpManager instance;
		return instance;
	}

	/**
	 * @brief Initialize the DFU SMP manager.
	 *
	 * @return ALIRO_NO_ERROR on success, error code otherwise.
	 */
	AliroError Init();

	/**
	 * @brief Toggle the DFU SMP manager.
	 *
	 * @return ALIRO_NO_ERROR on success, error code otherwise.
	 */
	void Toggle();

	/**
	 * @brief Check if the DFU SMP manager is enabled.
	 *
	 * @return true if the DFU SMP manager is enabled, false otherwise.
	 */
	bool IsSmpEnabled() const { return mIsAdvEnabled; }

	/**
	 * @brief Confirm the new image.
	 *
	 * @return ALIRO_NO_ERROR on success, error code otherwise.
	 */
	void ConfirmNewImage();

private:
	SmpManager() = default;
	SmpManager(const SmpManager &) = delete;
	SmpManager &operator=(const SmpManager &) = delete;
	SmpManager(SmpManager &&) = delete;
	SmpManager &operator=(SmpManager &&) = delete;

	/**
	 * @brief Start advertising the DFU SMP manager.
	 *
	 * @return ALIRO_NO_ERROR on success, error code otherwise.
	 */
	void StartAdvertising();

	/**
	 * @brief Stop advertising the DFU SMP manager.
	 *
	 * @return ALIRO_NO_ERROR on success, error code otherwise.
	 */
	void StopAdvertising();

	/**
	 * @brief Initialize the DFU SMP button.
	 *
	 * @return ALIRO_NO_ERROR on success, error code otherwise.
	 */
	AliroError InitButton();

	k_work mWork;

	bool mIsAdvEnabled{ false };
	bool mIsInitialized{ false };

	// DFU SMP service UUID.
	static constexpr std::array<uint8_t, BT_UUID_SIZE_128> kSmpUuid{ BT_UUID_DFU_SMP_SERVICE_VAL };

	// Advertising flags.
	static constexpr uint8_t kAdvertisingFlags{ BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR };

	// Advertising data storage.
	DoorLock::Interface::BleAdvertisingArbiter::Request mRequest{};
};

} // namespace Aliro::Dfu
