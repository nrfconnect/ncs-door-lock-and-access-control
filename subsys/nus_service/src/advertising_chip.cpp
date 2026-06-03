/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "advertising.h"

#include <platform/CHIPDeviceLayer.h>
#include <platform/Zephyr/BLEAdvertisingArbiter.h>

#include <bluetooth/services/nus.h>
#include <zephyr/bluetooth/bluetooth.h>

#include <array>
#include <cstring>

namespace DoorLock::NUSService::Advertising {

namespace {

namespace BleArbiter = chip::DeviceLayer::BLEAdvertisingArbiter;

constexpr std::array<uint8_t, BT_UUID_SIZE_128> kNusUuid{ BT_UUID_NUS_VAL };
constexpr uint8_t kAdvertisingFlags{ BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR };

BleArbiter::Request sRequest{};
bool sIsStarted;

std::array<bt_data, 2> sAdvertisingData;
std::array<bt_data, 1> sScanResponseData;

} // anonymous namespace

int InsertRequest(uint8_t priority, uint16_t minInterval, uint16_t maxInterval)
{
	sAdvertisingData[0] = BT_DATA(BT_DATA_FLAGS, &kAdvertisingFlags, sizeof(kAdvertisingFlags));
	sAdvertisingData[1] = BT_DATA(BT_DATA_UUID128_ALL, kNusUuid.data(), kNusUuid.size());

	const char *deviceName = bt_get_name();
	sScanResponseData[0] = BT_DATA(BT_DATA_NAME_COMPLETE, deviceName, static_cast<uint8_t>(strlen(deviceName)));

	sRequest = BleArbiter::Request{
		.priority = priority,
		.options = BT_LE_ADV_OPT_CONN,
		.minInterval = minInterval,
		.maxInterval = maxInterval,
		.advertisingData = chip::Span<bt_data>(sAdvertisingData),
		.scanResponseData = chip::Span<bt_data>(sScanResponseData),
		.onStarted =
			[](int rc) {
				if (rc == 0) {
					sIsStarted = true;
				}
			},
		.onStopped = []() { sIsStarted = false; },
	};

	chip::DeviceLayer::PlatformMgr().LockChipStack();
	const auto chipErr = BleArbiter::InsertRequest(sRequest);
	chip::DeviceLayer::PlatformMgr().UnlockChipStack();
	VerifyOrReturnValue(chipErr == CHIP_NO_ERROR, -EIO);

	return 0;
}

void CancelRequest()
{
	chip::DeviceLayer::PlatformMgr().LockChipStack();
	BleArbiter::CancelRequest(sRequest);
	sIsStarted = false;
	chip::DeviceLayer::PlatformMgr().UnlockChipStack();
}

bool IsStarted()
{
	return sIsStarted;
}

} // namespace DoorLock::NUSService::Advertising
