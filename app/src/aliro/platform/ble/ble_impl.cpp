/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

#include "ble_impl.h"

#include "aliro/utils.h"
#include "logger/platform_log.h"

#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/hci_vs.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(adv, CONFIG_NCS_DOOR_LOCK_APP_LOG_LEVEL);

#ifdef CONFIG_CHIP

#ifdef ReturnErrorOnFailure
#undef ReturnErrorOnFailure
#endif

#ifdef VerifyOrReturnValue
#undef VerifyOrReturnValue
#endif

#ifdef VerifyOrExit
#undef VerifyOrExit
#endif

#ifdef VerifyOrDie
#undef VerifyOrDie
#endif

#include <platform/CHIPDeviceLayer.h>
#include <platform/Zephyr/BLEAdvertisingArbiter.h>

using namespace ::chip;
using namespace ::chip::DeviceLayer;

namespace {
BLEAdvertisingArbiter::Request sAdvertisingRequest{};
} // namespace

#endif // CONFIG_CHIP

namespace Aliro::BleInterface {

AliroError BleAdvertisingImpl::Start(const BleTypes::AdvertisingService &service, uint8_t identity)
{
#ifdef CONFIG_CHIP

	if (mIsActive) {
		return ALIRO_NO_ERROR;
	}

#endif // CONFIG_CHIP

	mAdvertisingService = service;

	mAdvertisingData[kAdvertisingFlagsIndex] =
		BT_DATA(BT_DATA_FLAGS, &kAdvertisingFlags, sizeof(kAdvertisingFlags));
	mAdvertisingData[kAdvertisingServiceDataIndex] =
		BT_DATA(BT_DATA_SVC_DATA16, &mAdvertisingService, sizeof(mAdvertisingService));

	const char *name = bt_get_name();
	mScanResponseData[kScanResponseIndex] =
		BT_DATA(BT_DATA_NAME_COMPLETE, name, static_cast<uint8_t>(strlen(name)));

#ifdef CONFIG_CHIP

	sAdvertisingRequest.priority = kAdvertisingPriority;
	sAdvertisingRequest.options = kAdvertisingOptions;
	sAdvertisingRequest.minInterval = kIntervalMin;
	sAdvertisingRequest.maxInterval = kIntervalMax;
	sAdvertisingRequest.advertisingData = Span<bt_data>(mAdvertisingData);
	sAdvertisingRequest.scanResponseData = Span<bt_data>(mScanResponseData);

	sAdvertisingRequest.onStopped = []() {
		BleAdvertisingImpl::Instance().mIsActive = false;
		LOG_DBG("BLE advertising stopped");
	};

	PlatformMgr().LockChipStack();
	CHIP_ERROR ret = BLEAdvertisingArbiter::InsertRequest(sAdvertisingRequest);
	PlatformMgr().UnlockChipStack();

	VerifyOrReturnStatus(ret == CHIP_NO_ERROR, ALIRO_ERROR_INTERNAL,
			     LOG_ERR("Failed to insert BLE advertising request"));

#else

	bt_le_adv_param adv_param = BT_LE_ADV_PARAM_INIT(BT_LE_ADV_OPT_CONN, kIntervalMin, kIntervalMax, NULL);
	adv_param.id = identity;

	int error = bt_le_adv_start(&adv_param, mAdvertisingData.data(), mAdvertisingData.size(),
				    mScanResponseData.data(), mScanResponseData.size());
	VerifyOrReturnStatus(error == 0 || error == -EALREADY, ALIRO_ERROR_INTERNAL,
			     LOG_ERR("Cannot start advertising (err %d)", error));
	if (error == -EALREADY) {
		LOG_INF("Advertising already started");
	}

#endif // CONFIG_CHIP

	mIsActive = true;

	LOG_INF("BLE advertising started");
	return ALIRO_NO_ERROR;
}

AliroError BleAdvertisingImpl::UpdateData(const BleTypes::AdvertisingServiceData &serviceData)
{
	VerifyOrReturnStatus(mIsActive, ALIRO_INVALID_STATE, ALIRO_LOG_ERR("BLE advertising not started"));

	mAdvertisingService.mAdvertisingServiceData = serviceData;
	mAdvertisingData[kAdvertisingServiceDataIndex] =
		BT_DATA(BT_DATA_SVC_DATA16, &mAdvertisingService, sizeof(mAdvertisingService));

#ifdef CONFIG_CHIP

	sAdvertisingRequest.advertisingData = Span<bt_data>(mAdvertisingData);
	sAdvertisingRequest.scanResponseData = Span<bt_data>(mScanResponseData);

	PlatformMgr().LockChipStack();
	CHIP_ERROR ret = BLEAdvertisingArbiter::InsertRequest(sAdvertisingRequest);
	PlatformMgr().UnlockChipStack();
	VerifyOrReturnStatus(ret == CHIP_NO_ERROR, ALIRO_ERROR_INTERNAL,
			     ALIRO_LOG_ERR("Failed to update BLE advertising data"));

#else

	int error = bt_le_adv_update_data(mAdvertisingData.data(), mAdvertisingData.size(), mScanResponseData.data(),
					  mScanResponseData.size());
	VerifyOrReturnStatus(error == 0, ALIRO_ERROR_INTERNAL,
			     ALIRO_LOG_ERR("Cannot update advertising data (err %d)", error));

#endif // CONFIG_CHIP

	LOG_INF("BLE advertising service data updated");
	return ALIRO_NO_ERROR;
}

AliroError BleAdvertisingImpl::Stop()
{
	VerifyOrReturnStatus(mIsActive, ALIRO_INVALID_STATE, ALIRO_LOG_ERR("BLE advertising not started"));

#ifdef CONFIG_CHIP

	PlatformMgr().LockChipStack();
	BLEAdvertisingArbiter::CancelRequest(sAdvertisingRequest);
	PlatformMgr().UnlockChipStack();

#else

	int error = bt_le_adv_stop();
	VerifyOrReturnStatus(error == 0, ALIRO_ERROR_INTERNAL,
			     ALIRO_LOG_ERR("Cannot stop advertising (err %d)", error));

#endif // CONFIG_CHIP

	LOG_INF("BLE advertising stopped");

	mIsActive = false;
	return ALIRO_NO_ERROR;
}

AliroError BleAdvertisingImpl::GetTxPowerLevel(BleTypes::TxPowerLevel &txPowerLevel) const
{
#if defined(CONFIG_SOC_NRF5340_CPUAPP) || defined(CONFIG_BT_CTLR_TX_PWR_DYNAMIC_CONTROL)

	constexpr uint16_t kHandle{ 0 };
	constexpr uint8_t kHandleType{ BT_HCI_VS_LL_HANDLE_TYPE_ADV };

	net_buf *cmdBuff = bt_hci_cmd_alloc(K_FOREVER);
	VerifyOrReturnStatus(cmdBuff, ALIRO_NO_MEMORY, ALIRO_LOG_ERR("Cannot allocate memory for HCI command"));

	auto *cmd = static_cast<bt_hci_cp_vs_read_tx_power_level *>(
		net_buf_add(cmdBuff, sizeof(bt_hci_cp_vs_read_tx_power_level)));
	VerifyOrReturnStatus(cmd, ALIRO_ERROR_INTERNAL,
			     ALIRO_LOG_ERR("Cannot add data to the net buffer for HCI command"));

	cmd->handle = kHandle;
	cmd->handle_type = kHandleType;

	net_buf *respBuff{};
	int error = bt_hci_cmd_send_sync(BT_HCI_OP_VS_READ_TX_POWER_LEVEL, cmdBuff, &respBuff);
	VerifyOrReturnStatus(!error, ALIRO_ERROR_INTERNAL,
			     ALIRO_LOG_ERR("Failed to read Tx power level (error: %d)", error));

	const auto *resp = reinterpret_cast<bt_hci_rp_vs_read_tx_power_level *>(respBuff->data);
	txPowerLevel = resp->tx_power_level;

	net_buf_unref(respBuff);

#else // CONFIG_SOC_NRF5340_CPUAPP || CONFIG_BT_CTLR_TX_PWR_DYNAMIC_CONTROL

	txPowerLevel = static_cast<int8_t>(CONFIG_BT_CTLR_TX_PWR_DBM);

#endif // CONFIG_SOC_NRF5340_CPUAPP || CONFIG_BT_CTLR_TX_PWR_DYNAMIC_CONTROL

	return ALIRO_NO_ERROR;
}

} // namespace Aliro::BleInterface
