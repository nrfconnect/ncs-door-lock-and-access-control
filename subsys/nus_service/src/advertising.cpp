/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "advertising.h"

#include <advertising_arbiter/ble_advertising_arbiter.h>
#include <doorlock/utils/utils.h>

#include <bluetooth/services/nus.h>
#include <zephyr/bluetooth/bluetooth.h>

#include <array>
#include <cstring>

namespace DoorLock::NUSService::Advertising {

namespace {

namespace BleArbiter = DoorLock::Interface::BleAdvertisingArbiter;

BleArbiter::Request sRequest{};
bool sIsStarted;

constexpr std::array<uint8_t, BT_UUID_SIZE_128> kNusUuid{ BT_UUID_NUS_VAL };
constexpr uint8_t kAdvertisingFlags{ BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR };

} // anonymous namespace

int InsertRequest(uint8_t priority, uint16_t minInterval, uint16_t maxInterval)
{
	sRequest = BleArbiter::Request{ .mPriority = priority,
					.mOptions = BT_LE_ADV_OPT_CONN,
					.mMinInterval = minInterval,
					.mMaxInterval = maxInterval };

	sRequest.mAdvertisingData[0] = BT_DATA(BT_DATA_FLAGS, &kAdvertisingFlags, sizeof(kAdvertisingFlags));
	sRequest.mAdvertisingData[1] = BT_DATA(BT_DATA_UUID128_ALL, kNusUuid.data(), kNusUuid.size());

	const char *deviceName = bt_get_name();
	sRequest.mScanResponseData[0] =
		BT_DATA(BT_DATA_NAME_COMPLETE, deviceName, static_cast<uint8_t>(strlen(deviceName)));

	const int err = BleArbiter::InsertRequest(sRequest);
	VerifyOrReturnValue(err == 0, err);

	sIsStarted = true;

	return 0;
}

void CancelRequest()
{
	BleArbiter::CancelRequest(sRequest);
	sIsStarted = false;
}

bool IsStarted()
{
	return sIsStarted;
}

} // namespace DoorLock::NUSService::Advertising
