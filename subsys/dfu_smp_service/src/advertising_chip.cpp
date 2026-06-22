/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "advertising.h"

#include <platform/CHIPDeviceLayer.h>
#include <platform/Zephyr/BLEAdvertisingArbiter.h>

#include <bluetooth/services/dfu_smp.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/shell/shell.h>

#include <array>
#include <cstring>

namespace DoorLock::DfuSmpService::Advertising {

namespace {

namespace BleArbiter = chip::DeviceLayer::BLEAdvertisingArbiter;

constexpr std::array<uint8_t, BT_UUID_SIZE_128> kSmpUuid{ BT_UUID_DFU_SMP_SERVICE_VAL };
constexpr uint8_t kAdvertisingFlags{ BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR };

BleArbiter::Request sRequest{};

std::array<bt_data, 2> sAdvertisingData;
std::array<bt_data, 1> sScanResponseData;

bool sIsAdvertisingActive{ false };

} // namespace

int InsertRequest(uint8_t priority, uint16_t minInterval, uint16_t maxInterval)
{
	sAdvertisingData[0] = BT_DATA(BT_DATA_FLAGS, &kAdvertisingFlags, sizeof(kAdvertisingFlags));
	sAdvertisingData[1] = BT_DATA(BT_DATA_UUID128_ALL, kSmpUuid.data(), kSmpUuid.size());

	const char *deviceName = bt_get_name();
	sScanResponseData[0] = BT_DATA(BT_DATA_NAME_COMPLETE, deviceName, static_cast<uint8_t>(strlen(deviceName)));

	sRequest = BleArbiter::Request{
		.priority = priority,
		.options = BT_LE_ADV_OPT_CONN,
		.minInterval = minInterval,
		.maxInterval = maxInterval,
		.advertisingData = chip::Span<bt_data>(sAdvertisingData),
		.scanResponseData = chip::Span<bt_data>(sScanResponseData),
		.onStarted = nullptr,
		.onStopped = nullptr,
	};

	chip::DeviceLayer::PlatformMgr().LockChipStack();
	const auto chipErr = BleArbiter::InsertRequest(sRequest);
	chip::DeviceLayer::PlatformMgr().UnlockChipStack();
	VerifyOrReturnValue(chipErr == CHIP_NO_ERROR, -EIO);

	sIsAdvertisingActive = true;

	return 0;
}

void CancelRequest()
{
	chip::DeviceLayer::PlatformMgr().LockChipStack();
	BleArbiter::CancelRequest(sRequest);
	chip::DeviceLayer::PlatformMgr().UnlockChipStack();

	sIsAdvertisingActive = false;
}

bool IsActive()
{
	return sIsAdvertisingActive;
}

} // namespace DoorLock::DfuSmpService::Advertising
