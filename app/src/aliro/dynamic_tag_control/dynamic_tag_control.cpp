/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "dynamic_tag_control.h"

#include "aliro/platform/ble/ble_manager.h"
#include "aliro/storage/reader.h"

#include <aliro/aliro.h>
#include <aliro/utils.h>
#include <aliro_workqueue/aliro_workqueue.h>
#include <time_utils/time_utils.h>

#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(DynamicTagControl, CONFIG_DOOR_LOCK_APP_LOG_LEVEL);

namespace DoorLock::DynamicTagControl {
namespace {

using namespace Aliro;

void DynamicTagUpdateWorkHandler(k_work *);
K_WORK_DELAYABLE_DEFINE(sDynamicTagUpdateWork, DynamicTagUpdateWorkHandler);

AliroError Update(std::optional<uint32_t> unixTime)
{
	BleTypes::BleAddress address{};
	auto error = BleManager::Instance().GetAddress(address);
	VerifyOrReturnStatus(error == ALIRO_NO_ERROR, error, LOG_ERR("Failed to get BLE address: %d", error.ToInt()));

	BleTypes::TxPowerLevel txPower{};
	error = BleManager::Instance().GetTxPowerLevel(txPower);
	VerifyOrReturnStatus(error == ALIRO_NO_ERROR, error,
			     LOG_ERR("Failed to get TX power level: %d", error.ToInt()));

	Identifier readerIdentifier{};
	error = Storage::Reader::GetIdentifier(readerIdentifier);
	VerifyOrReturnStatus(error == ALIRO_NO_ERROR, error,
			     LOG_ERR("Failed to get Reader identifier: %d", error.ToInt()));

	BleTypes::BleExpiryTimestamp expiryTime{};
	if (unixTime.has_value()) {
		const uint32_t expiryTimestamp = *unixTime + CONFIG_DOOR_LOCK_DYNAMIC_TAG_CONTROL_EXPIRY_DURATION_S;
		// Convert to big-endian byte array as required by Aliro spec
		sys_put_be32(expiryTimestamp, expiryTime.data());
	} else {
		expiryTime = BleTypes::kExpiryTimeUnavailable;
	}

	BleTypes::AdvertisingServiceData advData{};
	error = AliroStack::GenerateAdvertisingData(advData, address, txPower, readerIdentifier,
						    BleTypes::AdvertisingServiceData::Notification::NoError,
						    expiryTime);
	VerifyOrReturnStatus(error == ALIRO_NO_ERROR, error,
			     LOG_ERR("Failed to generate advertising data: %d", error.ToInt()));

	if (BleManager::Instance().UpdateAdvertisingData(advData) != ALIRO_NO_ERROR) {
		error = BleManager::Instance().StartAdvertising(advData);
		VerifyOrReturnStatus(error == ALIRO_NO_ERROR, error,
				     LOG_ERR("Failed to start advertising: %d", error.ToInt()));
	}

	LOG_DBG("Dynamic Tag updated successfully");
	return ALIRO_NO_ERROR;
}

void DynamicTagUpdateWorkHandler(k_work *)
{
	const auto unixTime = TimeUtils::GetCurrentUnixTime();

	const auto error = Update(unixTime);
	if (error != ALIRO_NO_ERROR) {
		LOG_WRN("Periodic Dynamic Tag update failed: %d", error.ToInt());
	}

	if (unixTime.has_value()) {
		const int err = AliroWorkqueueReschedule(
			&sDynamicTagUpdateWork, K_SECONDS(CONFIG_DOOR_LOCK_DYNAMIC_TAG_CONTROL_EXPIRY_DURATION_S));
		VerifyOrReturn(err >= 0, LOG_ERR("Failed to reschedule Dynamic Tag update work: %d", err));
	}
}

} // namespace

int Start()
{
	const int err = AliroWorkqueueReschedule(&sDynamicTagUpdateWork, K_NO_WAIT);
	VerifyOrReturnValue(err >= 0, err, LOG_ERR("Failed to schedule Dynamic Tag update work: %d", err));
	return 0;
}

void Stop()
{
	LOG_INF("Stopping Dynamic Tag controller");
	static k_work_sync sync;
	k_work_cancel_delayable_sync(&sDynamicTagUpdateWork, &sync);
}

} // namespace DoorLock::DynamicTagControl
