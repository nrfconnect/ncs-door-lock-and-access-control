/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include <advertising_arbiter/ble_advertising_arbiter.h>

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
	 * @return 0 on success, error code otherwise.
	 */
	int Init();

	/**
	 * @brief Toggle the DFU SMP manager.
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
	 */
	void StartAdvertising();

	/**
	 * @brief Stop advertising the DFU SMP manager.
	 */
	void StopAdvertising();

	/**
	 * @brief Initialize the DFU SMP button.
	 *
	 * @return 0 on success, error code otherwise.
	 */
	int InitButton();

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
