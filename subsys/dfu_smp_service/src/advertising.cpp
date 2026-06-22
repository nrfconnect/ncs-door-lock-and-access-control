/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "advertising.h"

#include <advertising_arbiter/ble_advertising_arbiter.h>
#include <doorlock/utils/utils.h>

#include <bluetooth/services/dfu_smp.h>
#include <zephyr/bluetooth/bluetooth.h>

#include <array>
#include <cstring>

namespace DoorLock::DfuSmpService::Advertising {

namespace {

namespace BleArbiter = DoorLock::Interface::BleAdvertisingArbiter;

BleArbiter::Request sRequest{};

constexpr std::array<uint8_t, BT_UUID_SIZE_128> kSmpUuid{ BT_UUID_DFU_SMP_SERVICE_VAL };
constexpr uint8_t kAdvertisingFlags{ BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR };

bool sIsAdvertisingActive{ false };

} // namespace

int InsertRequest(uint8_t priority, uint16_t minInterval, uint16_t maxInterval)
{
	sRequest = BleArbiter::Request{ .mPriority = priority,
					.mOptions = BT_LE_ADV_OPT_CONN,
					.mMinInterval = minInterval,
					.mMaxInterval = maxInterval };

	sRequest.mAdvertisingData[0] = BT_DATA(BT_DATA_FLAGS, &kAdvertisingFlags, sizeof(kAdvertisingFlags));
	sRequest.mAdvertisingData[1] = BT_DATA(BT_DATA_UUID128_ALL, kSmpUuid.data(), kSmpUuid.size());

	const char *deviceName = bt_get_name();
	sRequest.mScanResponseData[0] =
		BT_DATA(BT_DATA_NAME_COMPLETE, deviceName, static_cast<uint8_t>(strlen(deviceName)));

	const int err = BleArbiter::InsertRequest(sRequest);
	VerifyOrReturnValue(err == 0, err);

	sIsAdvertisingActive = true;

	return err;
}

void CancelRequest()
{
	BleArbiter::CancelRequest(sRequest);
	sIsAdvertisingActive = false;
}

bool IsActive()
{
	return sIsAdvertisingActive;
}

} // namespace DoorLock::DfuSmpService::Advertising
