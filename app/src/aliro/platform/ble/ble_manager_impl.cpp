/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "ble_manager_impl.h"

#include "aliro/mutex_guard.h"

#ifdef CONFIG_ALIRO_BLE_UWB
#include "gatt_server/gatt_server.h"
#include "l2cap_server/l2cap_server.h"
#include "uwb/uwb.h"
#include "uwb_impl.h"
#endif // CONFIG_ALIRO_BLE_UWB

#ifdef CONFIG_DOOR_LOCK_BLE_NUS
#include "bt_nus/bt_nus.h"
using namespace Aliro::BtNus;
#endif // CONFIG_DOOR_LOCK_BLE_NUS

#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/hci_vs.h>
#include <zephyr/logging/log.h>
#include <zephyr/random/random.h>

#include <algorithm>
#include <cstring>

#ifdef CONFIG_CHIP

#include <platform/CHIPDeviceLayer.h>
#include <platform/Zephyr/BLEAdvertisingArbiter.h>

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

#endif // CONFIG_CHIP

#include "aliro/utils.h"

LOG_MODULE_REGISTER(BLEManagerImpl, CONFIG_NCS_ALIRO_BLE_LOG_LEVEL);

#ifdef CONFIG_CHIP

using namespace ::chip;
using namespace ::chip::DeviceLayer;

namespace {

BLEAdvertisingArbiter::Request sAdvertisingRequest{};

} // namespace

#endif // CONFIG_CHIP

namespace Aliro::BleInterface {

namespace {

K_MUTEX_DEFINE(sMutex);

constexpr uint8_t GetAdvertisingDataFieldType(BleTypes::AdvertisingDataFieldType type)
{
	switch (type) {
	case BleTypes::AdvertisingDataFieldType::Uuid16:
		return BT_DATA_SVC_DATA16;
	case BleTypes::AdvertisingDataFieldType::Uuid16All:
		return BT_DATA_UUID16_ALL;
	case BleTypes::AdvertisingDataFieldType::Uuid32:
		return BT_DATA_SVC_DATA32;
	case BleTypes::AdvertisingDataFieldType::Uuid32All:
		return BT_DATA_UUID32_ALL;
	case BleTypes::AdvertisingDataFieldType::Uuid128:
		return BT_DATA_SVC_DATA128;
	case BleTypes::AdvertisingDataFieldType::Uuid128All:
	default:
		return BT_DATA_UUID128_ALL;
	}
}

} // namespace

#ifndef CONFIG_CHIP

int BleManagerImpl::CreateRandomStaticAddress()
{
	// Generate a random static address for the default identity.
	// For nRF5340 this must be done before bt_enable() as after that updating the default identity is not possible.
	int error = sys_csrand_get(mAddress.a.val, sizeof(mAddress.a.val));
	VerifyOrReturnStatus(error == ALIRO_NO_ERROR, error, LOG_ERR("Cannot generate BLE random address"));

	mAddress.type = BT_ADDR_LE_RANDOM;
	BT_ADDR_SET_STATIC(&mAddress.a);

	return ALIRO_NO_ERROR;
}

void BleManagerImpl::Connected(bt_conn *connId, uint8_t error)
{
	mConnectionCount++;
	ResumeAdvertising();

	VerifyOrReturn(connId, LOG_ERR("Connection ID is null"));
	VerifyOrReturn(error == 0, LOG_ERR("Connection failed (error: %d, conn: %p)", error, connId));

	LOG_DBG("Connected (conn: %p)", connId);
#ifdef CONFIG_ALIRO_BLE_UWB
	Uwb::UltraWideBandImpl::Instance().BleTimeSync();
#endif // CONFIG_ALIRO_BLE_UWB

#ifdef CONFIG_DOOR_LOCK_BLE_NUS
	NUSService::Instance().Connected(connId, error);
#endif // CONFIG_DOOR_LOCK_BLE_NUS

	const bt_conn *refConn = bt_conn_ref(connId);
	VerifyOrReturn(refConn, LOG_ERR("Failed to reference connection (conn: %p)", connId));
}

void BleManagerImpl::Disconnected(bt_conn *connId, uint8_t reason)
{
	mConnectionCount--;
	bt_conn_unref(connId);

#ifdef CONFIG_DOOR_LOCK_BLE_NUS
	NUSService::Instance().Disconnected(connId, reason);
#endif // CONFIG_DOOR_LOCK_BLE_NUS

	LOG_DBG("Disconnected (reason: %d, conn: %p)", reason, connId);
}

// This callback is called when the BT is disconnected and also when the advertising is stopped.
void BleManagerImpl::Recycled()
{
	LOG_DBG("Connection recycled");

	ResumeAdvertising();
}

#ifdef CONFIG_BT_SMP

void BleManagerImpl::SecurityChanged([[maybe_unused]] bt_conn *connId, bt_security_t level, enum bt_security_err error)
{
	VerifyOrReturn(error == 0, LOG_ERR("Security failed (error: %d, conn: %p)", error, connId));

#ifdef CONFIG_DOOR_LOCK_BLE_NUS
	NUSService::Instance().SecurityChanged(connId, level, error);
#endif // CONFIG_DOOR_LOCK_BLE_NUS

	LOG_DBG("Security changed to level %u (conn: %p)", level, connId);
}

#endif // CONFIG_BT_SMP

void BleManagerImpl::ResumeAdvertising()
{
	// Allow to resume advertising only if it is enabled.
	VerifyOrReturn(IsAdvertising(), LOG_DBG("Skipped, advertising is disabled"));

	const auto workErr = k_work_submit(&mAdvResumeWork);
	VerifyOrReturn(workErr >= 0, LOG_ERR("Failed to submit work (error: %d)", workErr));
}

void BleManagerImpl::ResumeAdvertisingHandler()
{
	VerifyOrReturn(mConnectionCount < CONFIG_BT_MAX_CONN, LOG_DBG("Max connections reached"));

	auto error = StartAdvertising();
	VerifyOrReturn(error == ALIRO_NO_ERROR || error == ALIRO_INVALID_STATE,
		       LOG_ERR("Failed to resume advertising"));

	LOG_DBG("Advertising resumed");
}

#else // CONFIG_CHIP

AliroError BleManagerImpl::GetRandomStaticAddress()
{
	static_assert(CONFIG_BT_ID_MAX == 1, "CONFIG_BT_ID_MAX must be 1");

	size_t count{ CONFIG_BT_ID_MAX };
	bt_id_get(&mAddress, &count);

	VerifyOrReturnStatus(BT_ADDR_IS_STATIC(&mAddress.a), ALIRO_ERROR_INTERNAL, LOG_ERR("Address is not static"));

	return ALIRO_NO_ERROR;
}

#endif // CONFIG_CHIP

AliroError BleManagerImpl::SetAdvertisingData(const ConstData &data, BleTypes::AdvertisingDataFieldType type)
{
	VerifyOrReturnStatus(data.mLength <= mAdvertisingServiceData.size(), ALIRO_INVALID_ARGUMENT,
			     LOG_ERR("Invalid service data size"));

	std::copy_n(data.mData, data.mLength, mAdvertisingServiceData.data());
	mAdvertisingServiceDataSize = data.mLength;
	mAdvertisingDataFieldType = type;

	return ALIRO_NO_ERROR;
}

AliroError BleManagerImpl::Send(ConnectionHandle handle, Data data) const
{
#ifdef CONFIG_ALIRO_BLE_UWB

	VerifyOrReturnStatus(IsInitialized(), ALIRO_INVALID_STATE, LOG_ERR("BLE manager not initialized"));
	VerifyOrReturnStatus(handle, ALIRO_INVALID_ARGUMENT, LOG_ERR("Invalid connection handle"));

	return L2capServer::Instance().Send(static_cast<bt_conn *>(handle), data.mData, data.mLength);

#else // CONFIG_ALIRO_BLE_UWB

	return ALIRO_NO_ERROR;

#endif // CONFIG_ALIRO_BLE_UWB
}

AliroError BleManagerImpl::Disconnect(ConnectionHandle handle)
{
	VerifyOrReturnStatus(IsInitialized(), ALIRO_INVALID_STATE, LOG_ERR("BLE manager not initialized"));
	VerifyOrReturnStatus(handle, ALIRO_INVALID_ARGUMENT, LOG_ERR("Invalid connection handle"));

	bt_conn_info info{};
	VerifyOrReturnStatus(bt_conn_get_info(static_cast<bt_conn *>(handle), &info) == 0, ALIRO_ERROR_INTERNAL,
			     LOG_ERR("Failed to get connection info"));

	VerifyAndExit(info.state != BT_CONN_STATE_CONNECTED, LOG_DBG("No active connection found"));
	LOG_INF("Disconnecting (handle: %p)", handle);

	{
		int error = bt_conn_disconnect(static_cast<bt_conn *>(handle), BT_HCI_ERR_REMOTE_USER_TERM_CONN);
		VerifyOrReturnStatus(error == 0 || error == -ENOTCONN, ALIRO_ERROR_INTERNAL,
				     LOG_ERR("Failed to disconnect (error: %d)", error));
	}

exit:
	return ALIRO_NO_ERROR;
}

void BleManagerImpl::DisconnectAll()
{
	VerifyOrReturn(IsInitialized(), LOG_ERR("BLE manager not initialized"));

	bt_conn_foreach(
		BT_CONN_TYPE_ALL,
		[](bt_conn *conn, [[maybe_unused]] void *) {
			bt_conn_disconnect(conn, BT_HCI_ERR_REMOTE_USER_TERM_CONN);
		},
		nullptr);
}

AliroError BleManagerImpl::Init(const PlatformTransportCallbacks &callbacks)
{
	MutexGuard lock{ sMutex };

	VerifyOrReturnStatus(!IsInitialized(), ALIRO_INVALID_STATE, LOG_ERR("BLE manager already initialized"));

	mTransportCallbacks = callbacks;

#ifndef CONFIG_CHIP

	k_work_init(&mAdvResumeWork, []([[maybe_unused]] k_work *) { Instance().ResumeAdvertisingHandler(); });

	int idOrError = CreateRandomStaticAddress();
	VerifyOrReturnStatus(idOrError == 0, ALIRO_ERROR_INTERNAL, LOG_ERR("Cannot create static address"));

	idOrError = bt_id_create(&mAddress, nullptr);
	VerifyOrReturnStatus(idOrError >= 0, ALIRO_ERROR_INTERNAL,
			     LOG_ERR("Cannot create new ID (error: %d)", idOrError));

	idOrError = bt_enable(mReadyCb);
	VerifyOrReturnStatus(idOrError == 0, ALIRO_ERROR_INTERNAL,
			     LOG_ERR("Cannot enable BLE stack (error: %d)", idOrError));

	mConnCallbacks.connected = [](bt_conn *connId, uint8_t error) { Instance().Connected(connId, error); };
	mConnCallbacks.disconnected = [](bt_conn *connId, uint8_t reason) { Instance().Disconnected(connId, reason); };
	mConnCallbacks.recycled = []() { Instance().Recycled(); };

#ifdef CONFIG_BT_SMP

	mConnCallbacks.security_changed = [](bt_conn *connId, bt_security_t level, enum bt_security_err error) {
		Instance().SecurityChanged(connId, level, error);
	};

#endif // CONFIG_BT_SMP

	idOrError = bt_conn_cb_register(&mConnCallbacks);
	VerifyOrReturnStatus(idOrError == 0, ALIRO_ERROR_INTERNAL,
			     LOG_ERR("Cannot register BLE connection callbacks (error: %d)", idOrError));

	std::array<char, BT_ADDR_LE_STR_LEN> mAddressStr{};
	idOrError = bt_addr_le_to_str(&mAddress, mAddressStr.data(), mAddressStr.size());
	VerifyOrReturnStatus(idOrError >= 0, ALIRO_ERROR_INTERNAL,
			     LOG_ERR("Cannot convert address to string (error: %d)", idOrError));

	LOG_INF("Aliro BLE address: %s", mAddressStr.data());

#else // CONFIG_CHIP

	ReturnErrorOnFailure(GetRandomStaticAddress());

#endif // CONFIG_CHIP

#ifdef CONFIG_ALIRO_BLE_UWB

	AliroError error = L2capServer::Instance().Init();
	VerifyOrReturnStatus(error == ALIRO_NO_ERROR, error, LOG_ERR("Cannot initialize L2CAP server"));

	error = mGattServer.Init(L2capServer::Instance().GetSpsm());
	VerifyOrReturnStatus(error == ALIRO_NO_ERROR, error, LOG_ERR("Cannot initialize GATT server"));

	L2capServer::Callbacks l2capCallbacks = {
		.mOnConnected =
			[](bt_conn *conn) {
				VerifyAndCall(Instance().mTransportCallbacks.mOnTransportReady,
					      static_cast<ConnectionHandle>(conn));
			},
		.mOnDisconnected =
			[](bt_conn *conn) {
				VerifyAndCall(Instance().mTransportCallbacks.mOnTransportLoss,
					      static_cast<ConnectionHandle>(conn));
			},
		.mOnDataReceived =
			[](bt_conn *conn, uint8_t *data, size_t length) {
				VerifyAndCall(Instance().mTransportCallbacks.mOnDataReceived,
					      static_cast<ConnectionHandle>(conn), { data, length });
			},
	};
	L2capServer::Instance().SetCallbacks(l2capCallbacks);

#endif // CONFIG_ALIRO_BLE_UWB

	SetState(BleManagerState::Initialized);

	return ALIRO_NO_ERROR;
}

AliroError BleManagerImpl::GetAddress(BleTypes::BleAddress &address) const
{
	VerifyOrReturnStatus(IsInitialized(), ALIRO_INVALID_STATE, LOG_ERR("BLE manager not initialized"));
	std::copy_n(mAddress.a.val, BleTypes::kBleAddressSize, address.data());

	return ALIRO_NO_ERROR;
}

AliroError BleManagerImpl::GetTxPowerLevel(BleTypes::TxPowerLevel &txPowerLevel) const
{
	VerifyOrReturnStatus(IsInitialized(), ALIRO_INVALID_STATE, LOG_ERR("BLE manager not initialized"));

#if defined(CONFIG_SOC_NRF5340_CPUAPP) || defined(CONFIG_BT_CTLR_TX_PWR_DYNAMIC_CONTROL)

	constexpr uint16_t kHandle{ 0 };
	constexpr uint8_t kHandleType{ BT_HCI_VS_LL_HANDLE_TYPE_ADV };

	net_buf *cmdBuff = bt_hci_cmd_alloc(K_FOREVER);
	VerifyOrReturnStatus(cmdBuff, ALIRO_NO_MEMORY, LOG_ERR("Cannot allocate memory for HCI command"));

	auto *cmd = static_cast<bt_hci_cp_vs_read_tx_power_level *>(
		net_buf_add(cmdBuff, sizeof(bt_hci_cp_vs_read_tx_power_level)));
	VerifyOrReturnStatus(cmd, ALIRO_ERROR_INTERNAL, LOG_ERR("Cannot add data to the net buffer for HCI command"));

	cmd->handle = kHandle;
	cmd->handle_type = kHandleType;

	net_buf *respBuff{};
	int error = bt_hci_cmd_send_sync(BT_HCI_OP_VS_READ_TX_POWER_LEVEL, cmdBuff, &respBuff);
	VerifyOrReturnStatus(!error, ALIRO_ERROR_INTERNAL, LOG_ERR("Failed to read Tx power level (error: %d)", error));

	VerifyOrReturnStatus(respBuff->len >= sizeof(bt_hci_rp_vs_read_tx_power_level), ALIRO_ERROR_INTERNAL,
			     LOG_ERR("Invalid response buffer size"));

	bt_hci_rp_vs_read_tx_power_level resp{};
	std::memcpy(&resp, respBuff->data, sizeof(bt_hci_rp_vs_read_tx_power_level));
	txPowerLevel = resp.tx_power_level;

	net_buf_unref(respBuff);

#else // CONFIG_SOC_NRF5340_CPUAPP || CONFIG_BT_CTLR_TX_PWR_DYNAMIC_CONTROL

	txPowerLevel = static_cast<int8_t>(CONFIG_BT_CTLR_TX_PWR_DBM);

#endif // CONFIG_SOC_NRF5340_CPUAPP || CONFIG_BT_CTLR_TX_PWR_DYNAMIC_CONTROL

	return ALIRO_NO_ERROR;
}

size_t BleManagerImpl::GetMaxSessions() const
{
#ifdef CONFIG_ALIRO_BLE_UWB_MAX_SESSIONS

	return CONFIG_ALIRO_BLE_UWB_MAX_SESSIONS;

#else // CONFIG_ALIRO_BLE_UWB_MAX_SESSIONS

	return 0;

#endif // CONFIG_ALIRO_BLE_UWB_MAX_SESSIONS
}

AliroError BleManagerImpl::StartAdvertising(const ConstData &data, BleTypes::AdvertisingDataFieldType type)
{
	VerifyOrReturnStatus(IsInitialized(), ALIRO_INVALID_STATE, LOG_ERR("BLE manager not initialized"));

	ReturnErrorOnFailure(SetAdvertisingData(data, type));

	return StartAdvertising();
}

AliroError BleManagerImpl::StartAdvertising()
{
	MutexGuard lock{ sMutex };

	VerifyOrReturnStatus(IsInitialized(), ALIRO_INVALID_STATE, LOG_ERR("BLE manager not initialized"));

	mAdvertisingData[kAdvertisingFlagsIndex] =
		BT_DATA(BT_DATA_FLAGS, &kAdvertisingFlags, sizeof(kAdvertisingFlags));

	mAdvertisingData[kAdvertisingServiceDataIndex] =
		BT_DATA(GetAdvertisingDataFieldType(mAdvertisingDataFieldType), mAdvertisingServiceData.data(),
			mAdvertisingServiceDataSize);

	const char *name = bt_get_name();
	mScanResponseData[kScanResponseIndex] =
		BT_DATA(BT_DATA_NAME_COMPLETE, name, static_cast<uint8_t>(strlen(name)));

#ifndef CONFIG_CHIP

	bt_le_adv_param advParam = BT_LE_ADV_PARAM_INIT(BT_LE_ADV_OPT_CONN, kIntervalMin, kIntervalMax, NULL);

	int error = bt_le_adv_start(&advParam, mAdvertisingData.data(), mAdvertisingData.size(),
				    mScanResponseData.data(), mScanResponseData.size());
	VerifyOrReturnStatus(error == 0 || error == -EALREADY, ALIRO_ERROR_INTERNAL,
			     LOG_ERR("Cannot start advertising (err %d)", error));
	if (error == -EALREADY) {
		LOG_INF("Advertising already started");
		return ALIRO_NO_ERROR;
	}

#else // CONFIG_CHIP

	sAdvertisingRequest.priority = kAdvertisingPriority;
	sAdvertisingRequest.options = kAdvertisingOptions;
	sAdvertisingRequest.minInterval = kIntervalMin;
	sAdvertisingRequest.maxInterval = kIntervalMax;
	sAdvertisingRequest.advertisingData = Span<bt_data>(mAdvertisingData);
	sAdvertisingRequest.scanResponseData = Span<bt_data>(mScanResponseData);

	sAdvertisingRequest.onStopped = []() {
		Instance().SetState(BleManagerState::Initialized);
		LOG_DBG("BLE advertising stopped");
	};

	PlatformMgr().LockChipStack();
	CHIP_ERROR ret = BLEAdvertisingArbiter::InsertRequest(sAdvertisingRequest);
	PlatformMgr().UnlockChipStack();

	VerifyOrReturnStatus(ret == CHIP_NO_ERROR, ALIRO_ERROR_INTERNAL,
			     LOG_ERR("Failed to insert BLE advertising request"));

#endif // CONFIG_CHIP

	SetState(BleManagerState::Advertising);

	LOG_INF("BLE advertising started");
	LOG_HEXDUMP_DBG(mAdvertisingServiceData.data(), mAdvertisingServiceDataSize, "Advertising data:");
	return ALIRO_NO_ERROR;
}

AliroError BleManagerImpl::UpdateAdvertisingData(const ConstData &data, BleTypes::AdvertisingDataFieldType type)
{
	VerifyOrReturnStatus(IsAdvertising(), ALIRO_INVALID_STATE, LOG_ERR("BLE manager not advertising"));

	ReturnErrorOnFailure(SetAdvertisingData(data, type));

	mAdvertisingData[kAdvertisingServiceDataIndex] =
		BT_DATA(GetAdvertisingDataFieldType(mAdvertisingDataFieldType), mAdvertisingServiceData.data(),
			mAdvertisingServiceDataSize);

#ifndef CONFIG_CHIP

	int error = bt_le_adv_update_data(mAdvertisingData.data(), mAdvertisingData.size(), mScanResponseData.data(),
					  mScanResponseData.size());
	VerifyOrReturnStatus(error == 0, ALIRO_ERROR_INTERNAL,
			     LOG_ERR("Cannot update advertising data (err %d)", error));

#else // CONFIG_CHIP

	sAdvertisingRequest.advertisingData = Span<bt_data>(mAdvertisingData);
	sAdvertisingRequest.scanResponseData = Span<bt_data>(mScanResponseData);

	PlatformMgr().LockChipStack();
	CHIP_ERROR ret = BLEAdvertisingArbiter::InsertRequest(sAdvertisingRequest);
	PlatformMgr().UnlockChipStack();
	VerifyOrReturnStatus(ret == CHIP_NO_ERROR, ALIRO_ERROR_INTERNAL,
			     LOG_ERR("Failed to update BLE advertising data"));

#endif // CONFIG_CHIP

	LOG_INF("BLE advertising service data updated");
	LOG_HEXDUMP_DBG(mAdvertisingServiceData.data(), mAdvertisingServiceDataSize, "Advertising data:");
	return ALIRO_NO_ERROR;
}

AliroError BleManagerImpl::StopAdvertising()
{
	MutexGuard lock{ sMutex };
	VerifyOrReturnStatus(IsAdvertising(), ALIRO_INVALID_STATE, LOG_ERR("BLE manager not advertising"));

#ifndef CONFIG_CHIP

	int error = bt_le_adv_stop();
	VerifyOrReturnStatus(error == 0, ALIRO_ERROR_INTERNAL, LOG_ERR("Cannot stop advertising (err %d)", error));

#else // CONFIG_CHIP

	PlatformMgr().LockChipStack();
	BLEAdvertisingArbiter::CancelRequest(sAdvertisingRequest);
	PlatformMgr().UnlockChipStack();

#endif // CONFIG_CHIP

	SetState(BleManagerState::Initialized);

	LOG_INF("BLE advertising stopped");

	return ALIRO_NO_ERROR;
}

} // namespace Aliro::BleInterface
