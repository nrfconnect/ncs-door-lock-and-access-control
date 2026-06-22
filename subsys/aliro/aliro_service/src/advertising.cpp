/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "advertising.h"

#include "utils.h"

#include <aliro/aliro.h>
#include <doorlock/utils/utils.h>
#include <reader_storage/reader.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/logging/log.h>

#include <cerrno>
#include <cstdint>
#include <cstring>
#include <iterator>

LOG_MODULE_DECLARE(AliroService, CONFIG_DOOR_LOCK_ALIRO_BLE_SERVICE_LOG_LEVEL);

namespace DoorLock::AliroService::Advertising {

namespace {

Aliro::BleTypes::BleAddress sAddress;
bt_le_ext_adv *sAdvertiser{};

} // namespace

int Init(const Aliro::BleTypes::BleAddress &address)
{
	sAddress = address;

	bt_le_adv_param params = BT_LE_ADV_PARAM_INIT(BT_LE_ADV_OPT_CONN | BT_LE_ADV_OPT_USE_IDENTITY,
						      BT_GAP_ADV_FAST_INT_MIN_2, BT_GAP_ADV_FAST_INT_MAX_2, nullptr);
	params.id = Utils::kAliroBtId;

	const int err = bt_le_ext_adv_create(&params, nullptr, &sAdvertiser);
	VerifyOrReturnValue(err == 0, err, LOG_ERR("Failed to create Aliro advertising set: %d", err));

	return 0;
}

int SetData(Aliro::BleTypes::AdvertisingServiceData::Notification notification,
	    Aliro::BleTypes::BleExpiryTimestamp expiryTime)
{
	Aliro::Identifier readerIdentifier{};
	AliroError ec = DoorLock::ReaderStorage::GetIdentifier(readerIdentifier);
	VerifyOrReturnValue(ec == ALIRO_NO_ERROR, -EIO, LOG_ERR("Failed to get Reader identifier: %d", ec.ToInt()));

	Aliro::BleTypes::TxPowerLevel txPower{};
	int err = Utils::GetTxPower(txPower);
	VerifyOrReturnValue(err == 0, err);

	Aliro::BleTypes::AdvertisingService advData{};
	ec = Aliro::AliroStack::Instance().GenerateAdvertisingData(advData.mAdvertisingServiceData, sAddress, txPower,
								   readerIdentifier, notification, expiryTime);
	VerifyOrReturnValue(ec == ALIRO_NO_ERROR, -EIO, LOG_ERR("Failed to generate advertising data: %d", ec.ToInt()));

	constexpr uint8_t kAdvertisingFlagsData[]{ BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR };
	const bt_data sAdvertisingData[2]{ BT_DATA(BT_DATA_FLAGS, kAdvertisingFlagsData, sizeof(kAdvertisingFlagsData)),
					   BT_DATA(BT_DATA_SVC_DATA16, &advData,
						   static_cast<uint8_t>(sizeof(advData))) };

	const char *name = bt_get_name();
	const bt_data sScanResponseData[]{ BT_DATA(BT_DATA_NAME_COMPLETE, name, static_cast<uint8_t>(strlen(name))) };

	err = bt_le_ext_adv_set_data(sAdvertiser, sAdvertisingData, std::size(sAdvertisingData), sScanResponseData,
				     std::size(sScanResponseData));
	VerifyOrReturnValue(err == 0, err, LOG_ERR("Failed to set advertising data: %d", err));

	return 0;
}

int Start()
{
	const int err = bt_le_ext_adv_start(sAdvertiser, BT_LE_EXT_ADV_START_DEFAULT);
	VerifyOrReturnValue(err == 0 || err == -EALREADY, err, LOG_ERR("Failed to start Aliro advertising: %d", err));

	LOG_INF("Aliro BLE advertising started");

	return 0;
}

int Stop()
{
	const int err = bt_le_ext_adv_stop(sAdvertiser);
	VerifyOrReturnValue(err == 0 || err == -EALREADY, err, LOG_ERR("Failed to stop Aliro advertising: %d", err));

	return 0;
}

} // namespace DoorLock::AliroService::Advertising
