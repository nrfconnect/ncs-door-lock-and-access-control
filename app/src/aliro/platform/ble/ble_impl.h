/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

#pragma once

#include "aliro/ble_types.h"
#include "transport/ble/ble_iface.h"

#include <zephyr/bluetooth/bluetooth.h>

#include <array>

namespace Aliro::BleInterface {

/**
 * @class BleAdvertisingImpl
 * @brief Implementation of BLE advertising interface.
 *
 * This class provides concrete implementation of BLE advertising functionality
 * and integrates with Matter's BLEAdvertisingArbiter.
 */
class BleAdvertisingImpl : public BleAdvertisingIfc {
public:
	/**
	 * @brief Get the singleton instance of BLE advertising implementation
	 *
	 * @return Reference to the singleton instance
	 */
	static BleAdvertisingImpl &Instance()
	{
		static BleAdvertisingImpl sInstance;
		return sInstance;
	}

	// BleAdvertisingIfc interface implementation
	bool IsActive() const override { return mIsActive; }
	AliroError Start(const BleTypes::AdvertisingService &service, uint8_t identity) override;
	AliroError UpdateData(const BleTypes::AdvertisingServiceData &serviceData) override;
	AliroError Stop() override;
	AliroError GetTxPowerLevel(BleTypes::TxPowerLevel &txPowerLevel) const override;

private:
	BleAdvertisingImpl() = default;
	~BleAdvertisingImpl() final = default;
	BleAdvertisingImpl(const BleAdvertisingImpl &) = delete;
	BleAdvertisingImpl &operator=(const BleAdvertisingImpl &) = delete;
	BleAdvertisingImpl(BleAdvertisingImpl &&) = delete;
	BleAdvertisingImpl &operator=(BleAdvertisingImpl &&) = delete;

	static constexpr size_t kAdvertisingDataSize{ 2 };
	static constexpr size_t kScanResponseSize{ 1 };
	static constexpr uint32_t kAdvertisingPriority{ 1 };
	static constexpr uint8_t kAdvertisingFlags{ BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR };
	static constexpr uint32_t kIntervalMin{ BT_GAP_ADV_FAST_INT_MIN_2 };
	static constexpr uint32_t kIntervalMax{ BT_GAP_ADV_FAST_INT_MAX_2 };
	static constexpr uint32_t kAdvertisingOptions{ BT_LE_ADV_OPT_CONN };
	static constexpr size_t kAdvertisingFlagsIndex{ 0 };
	static constexpr size_t kAdvertisingServiceDataIndex{ 1 };
	static constexpr size_t kScanResponseIndex{ 0 };

	BleTypes::AdvertisingService mAdvertisingService{};
	bool mIsActive{};

	using AdvertisingData = std::array<bt_data, kAdvertisingDataSize>;
	using ScanResponseData = std::array<bt_data, kScanResponseSize>;
	AdvertisingData mAdvertisingData{};
	ScanResponseData mScanResponseData{};
};

} // namespace Aliro::BleInterface
