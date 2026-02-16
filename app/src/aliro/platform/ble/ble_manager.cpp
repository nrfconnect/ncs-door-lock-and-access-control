/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "ble_manager.h"

#include "aliro/aliro.h"
#include "aliro/aliro_work/aliro_work.h"
#include "aliro/mutex_guard.h"
#include "ble_advertising_arbiter.h"

#ifdef CONFIG_DOOR_LOCK_BLE_UWB
#include "gatt_server/gatt_server.h"
#include "l2cap_server/l2cap_server.h"
#include "uwb.h"
#include "uwb_impl.h"
#endif // CONFIG_DOOR_LOCK_BLE_UWB

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

#include "aliro/utils.h"

LOG_MODULE_REGISTER(BleManager, CONFIG_DOOR_LOCK_BLE_LOG_LEVEL);

namespace Aliro {

namespace {

K_MUTEX_DEFINE(sMutex);

namespace BleAdvArbiter = DoorLock::Interface::BleAdvertisingArbiter;

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

int BleManager::CreateRandomStaticAddress()
{
	// Generate a random static address for the default identity.
	// For nRF5340 this must be done before bt_enable() as after that updating the default identity is not possible.
	int error = sys_csrand_get(mAddress.a.val, sizeof(mAddress.a.val));
	VerifyOrReturnStatus(error == ALIRO_NO_ERROR, error, LOG_ERR("Cannot generate BLE random address"));

	mAddress.type = BT_ADDR_LE_RANDOM;
	BT_ADDR_SET_STATIC(&mAddress.a);

	return ALIRO_NO_ERROR;
}

void BleManager::Connected(bt_conn *connId, uint8_t error)
{
	mConnectionCount++;
	ResumeAdvertising();

	VerifyOrReturn(connId, LOG_ERR("Connection handle is null"));
	VerifyOrReturn(error == 0, LOG_ERR("Connection failed (error: %d, conn: %p)", error, connId));

	LOG_DBG("Connected (conn: %p)", connId);
#ifdef CONFIG_DOOR_LOCK_BLE_UWB
	Uwb::UltraWideBandImpl::Instance().BleTimeSync();
#endif // CONFIG_DOOR_LOCK_BLE_UWB

#ifdef CONFIG_DOOR_LOCK_BLE_NUS
	NUSService::Instance().Connected(connId, error);
#endif // CONFIG_DOOR_LOCK_BLE_NUS

	const bt_conn *refConn = bt_conn_ref(connId);
	VerifyOrReturn(refConn, LOG_ERR("Failed to reference connection (conn: %p)", connId));
}

void BleManager::Disconnected(bt_conn *connId, uint8_t reason)
{
	mConnectionCount--;

	bt_conn_unref(connId);

#ifdef CONFIG_DOOR_LOCK_BLE_NUS
	NUSService::Instance().Disconnected(connId, reason);
#endif // CONFIG_DOOR_LOCK_BLE_NUS

	LOG_DBG("Disconnected (reason: %d, conn: %p)", reason, connId);
}

// This callback is called when the BT is disconnected and also when the advertising is stopped.
void BleManager::Recycled()
{
	LOG_DBG("Connection recycled");

	ResumeAdvertising();
}

#ifdef CONFIG_BT_SMP

void BleManager::SecurityChanged(bt_conn *connId, bt_security_t level, bt_security_err error)
{
	VerifyOrReturn(error == BT_SECURITY_ERR_SUCCESS,
		       LOG_WRN("Security failed (error: %d, conn: %p)", static_cast<int>(error), connId));

#ifdef CONFIG_DOOR_LOCK_BLE_NUS
	NUSService::Instance().SecurityChanged(connId, level, error);
#endif // CONFIG_DOOR_LOCK_BLE_NUS

	LOG_DBG("Security changed to level %u (conn: %p)", level, connId);
}

#endif // CONFIG_BT_SMP

void BleManager::ResumeAdvertising()
{
	// Allow to resume advertising only if it is enabled.
	VerifyOrReturn(IsAdvertising(), LOG_DBG("Skipped, advertising is disabled"));

	const auto workErr = AliroWorkSubmit(&mAdvResumeWork);
	VerifyOrReturn(workErr >= 0, LOG_ERR("Failed to submit work (error: %d)", workErr));
}

void BleManager::ResumeAdvertisingHandler()
{
	VerifyOrReturn(mConnectionCount < CONFIG_BT_MAX_CONN, LOG_DBG("Max connections reached"));

	auto error = RestartAdvertising();
	VerifyOrReturn(error == ALIRO_NO_ERROR || error == ALIRO_INVALID_STATE,
		       LOG_ERR("Failed to resume advertising"));

	LOG_DBG("Advertising resumed");
}

#else // CONFIG_CHIP

AliroError BleManager::GetRandomStaticAddress()
{
	static_assert(CONFIG_BT_ID_MAX == 1, "CONFIG_BT_ID_MAX must be 1");

	size_t count{ CONFIG_BT_ID_MAX };
	bt_id_get(&mAddress, &count);

	VerifyOrReturnStatus(BT_ADDR_IS_STATIC(&mAddress.a), ALIRO_ERROR_INTERNAL, LOG_ERR("Address is not static"));

	return ALIRO_NO_ERROR;
}

#endif // CONFIG_CHIP

AliroError BleManager::SetAdvertisingData(const ConstData &data, BleTypes::AdvertisingDataFieldType type)
{
	VerifyOrReturnStatus(data.mLength <= mAdvertisingServiceData.size(), ALIRO_INVALID_ARGUMENT,
			     LOG_ERR("Invalid service data size"));

	std::copy_n(data.mData, data.mLength, mAdvertisingServiceData.data());
	mAdvertisingServiceDataSize = data.mLength;
	mAdvertisingDataFieldType = type;

	return ALIRO_NO_ERROR;
}

AliroError BleManager::Send(ConnectionHandle handle, Data data) const
{
#ifdef CONFIG_DOOR_LOCK_BLE_UWB

	VerifyOrReturnStatus(IsInitialized(), ALIRO_INVALID_STATE, LOG_ERR("BLE manager not initialized"));
	VerifyOrReturnStatus(handle.IsBle(), ALIRO_INVALID_ARGUMENT, LOG_ERR("Invalid connection handle"));

	return L2capServer::Instance().Send(handle.GetBtConn(), data.mData, data.mLength);

#else // CONFIG_DOOR_LOCK_BLE_UWB

	return ALIRO_NO_ERROR;

#endif // CONFIG_DOOR_LOCK_BLE_UWB
}

AliroError BleManager::Terminate(ConnectionHandle handle)
{
	VerifyOrReturnStatus(IsInitialized(), ALIRO_INVALID_STATE, LOG_ERR("BLE manager not initialized"));
	VerifyOrReturnStatus(handle.IsBle(), ALIRO_INVALID_ARGUMENT, LOG_ERR("Invalid connection handle"));

	bt_conn_info info{};
	VerifyOrReturnStatus(bt_conn_get_info(handle.GetBtConn(), &info) == 0, ALIRO_ERROR_INTERNAL,
			     LOG_ERR("Failed to get connection info"));

	VerifyAndExit(info.state != BT_CONN_STATE_CONNECTED, LOG_DBG("No active connection found"));
	LOG_INF("Disconnecting (handle: %p)", handle.GetRaw());

	{
		int error = bt_conn_disconnect(handle.GetBtConn(), BT_HCI_ERR_REMOTE_USER_TERM_CONN);
		VerifyOrReturnStatus(error == 0 || error == -ENOTCONN, ALIRO_ERROR_INTERNAL,
				     LOG_ERR("Failed to disconnect (error: %d)", error));
	}

exit:
	return ALIRO_NO_ERROR;
}

AliroError BleManager::DisconnectAll()
{
	VerifyOrReturnStatus(IsInitialized(), ALIRO_INVALID_STATE, LOG_ERR("BLE manager not initialized"));

	bt_conn_foreach(
		BT_CONN_TYPE_ALL,
		[](bt_conn *conn, [[maybe_unused]] void *) {
			bt_conn_disconnect(conn, BT_HCI_ERR_REMOTE_USER_TERM_CONN);
		},
		nullptr);

	return ALIRO_NO_ERROR;
}

AliroError BleManager::Init()
{
	MutexGuard lock{ sMutex };

	VerifyOrReturnStatus(!IsInitialized(), ALIRO_INVALID_STATE, LOG_ERR("BLE manager already initialized"));

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

#ifdef CONFIG_DOOR_LOCK_BLE_UWB

	AliroError error = L2capServer::Instance().Init();
	VerifyOrReturnStatus(error == ALIRO_NO_ERROR, error, LOG_ERR("Cannot initialize L2CAP server"));

	L2capServer::Callbacks l2capCallbacks = {
		.mOnConnected =
			[](bt_conn *conn) {
				AliroStack::Instance().CreateSession(ConnectionHandle::Ble(conn));
			},
		.mOnDisconnected =
			[](bt_conn *conn) {
				AliroStack::Instance().DestroySession(ConnectionHandle::Ble(conn));
			},
		.mOnDataReceived =
			[](bt_conn *conn, uint8_t *data, size_t length) {
				AliroStack::Instance().HandleSessionData(ConnectionHandle::Ble(conn),
									 { data, length });
			},
	};

	L2capServer::Instance().SetCallbacks(l2capCallbacks);

	error = mGattServer.Init(L2capServer::Instance());
	VerifyOrReturnStatus(error == ALIRO_NO_ERROR, error, LOG_ERR("Cannot initialize GATT server"));

#endif // CONFIG_DOOR_LOCK_BLE_UWB

	SetState(BleManagerState::Initialized);

	return ALIRO_NO_ERROR;
}

AliroError BleManager::GetAddress(BleTypes::BleAddress &address) const
{
	VerifyOrReturnStatus(IsInitialized(), ALIRO_INVALID_STATE, LOG_ERR("BLE manager not initialized"));
	std::copy_n(mAddress.a.val, BleTypes::kBleAddressSize, address.data());

	return ALIRO_NO_ERROR;
}

AliroError BleManager::GetTxPowerLevel(BleTypes::TxPowerLevel &txPowerLevel) const
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

size_t BleManager::GetMaxSessions() const
{
#ifdef CONFIG_DOOR_LOCK_BLE_UWB_MAX_SESSIONS

	return CONFIG_DOOR_LOCK_BLE_UWB_MAX_SESSIONS;

#else // CONFIG_DOOR_LOCK_BLE_UWB_MAX_SESSIONS

	return 0;

#endif // CONFIG_DOOR_LOCK_BLE_UWB_MAX_SESSIONS
}

AliroError BleManager::StartAdvertising(const BleTypes::AdvertisingServiceData &data)
{
	BleTypes::AdvertisingService advertisingService{};
	advertisingService.mAdvertisingServiceData = data;

	return StartAdvertising({ reinterpret_cast<const uint8_t *>(&advertisingService), sizeof(advertisingService) },
				BleTypes::AdvertisingDataFieldType::Uuid16);
}

AliroError BleManager::StartAdvertising(const ConstData &data, BleTypes::AdvertisingDataFieldType type)
{
	VerifyOrReturnStatus(IsInitialized(), ALIRO_INVALID_STATE, LOG_ERR("BLE manager not initialized"));

	ReturnErrorOnFailure(SetAdvertisingData(data, type));

	return RestartAdvertising();
}

AliroError BleManager::RestartAdvertising()
{
	MutexGuard lock{ sMutex };

	VerifyOrReturnStatus(IsInitialized(), ALIRO_INVALID_STATE, LOG_ERR("BLE manager not initialized"));

	mBleAdvertisingRequest = BleAdvArbiter::Request{ .mOptions = kAdvertisingOptions,
							 .mMinInterval = kIntervalMin,
							 .mMaxInterval = kIntervalMax };

	// Set advertising data buffers
	mBleAdvertisingRequest.mAdvertisingData[0] =
		BT_DATA(BT_DATA_FLAGS, &kAdvertisingFlags, sizeof(kAdvertisingFlags));

	mBleAdvertisingRequest.mAdvertisingData[1] =
		BT_DATA(GetAdvertisingDataFieldType(mAdvertisingDataFieldType), mAdvertisingServiceData.data(),
			mAdvertisingServiceDataSize);

	const char *name = bt_get_name();
	mBleAdvertisingRequest.mScanResponseData[0] =
		BT_DATA(BT_DATA_NAME_COMPLETE, name, static_cast<uint8_t>(strlen(name)));

	AliroError err = BleAdvArbiter::InsertRequest(BleAdvArbiter::Component::Aliro, mBleAdvertisingRequest);
	VerifyOrReturnStatus(err == ALIRO_NO_ERROR, err, LOG_ERR("Failed to insert Aliro advertising request"));

	SetState(BleManagerState::Advertising);

	LOG_HEXDUMP_DBG(mAdvertisingServiceData.data(), mAdvertisingServiceDataSize, "Advertising data:");
	return ALIRO_NO_ERROR;
}

AliroError BleManager::UpdateAdvertisingData(const BleTypes::AdvertisingServiceData &data)
{
	BleTypes::AdvertisingService advertisingService{};
	advertisingService.mAdvertisingServiceData = data;

	return UpdateAdvertisingData({ reinterpret_cast<const uint8_t *>(&advertisingService),
				       sizeof(advertisingService) },
				     BleTypes::AdvertisingDataFieldType::Uuid16);
}

AliroError BleManager::UpdateAdvertisingData(const ConstData &data, BleTypes::AdvertisingDataFieldType type)
{
	VerifyOrReturnStatus(IsAdvertising(), ALIRO_INVALID_STATE, LOG_ERR("BLE manager not advertising"));

	ReturnErrorOnFailure(SetAdvertisingData(data, type));

	return RestartAdvertising();
}

AliroError BleManager::StopAdvertising()
{
	MutexGuard lock{ sMutex };
	VerifyOrReturnStatus(IsAdvertising(), ALIRO_INVALID_STATE, LOG_ERR("BLE manager not advertising"));

	BleAdvArbiter::CancelRequest(BleAdvArbiter::Component::Aliro);

	SetState(BleManagerState::Initialized);

	return ALIRO_NO_ERROR;
}

ProtocolVersion BleManager::GetProtocolVersion(ConnectionHandle handle) const
{
#ifdef CONFIG_DOOR_LOCK_BLE_UWB

	VerifyOrReturnValue(IsInitialized(), BleTypes::kInvalidProtocolVersion, LOG_ERR("BLE manager not initialized"));
	VerifyOrReturnValue(handle.IsBle(), BleTypes::kInvalidProtocolVersion, LOG_ERR("Invalid connection handle"));

	return L2capServer::Instance().GetBleUwbProtocolVersion(handle.GetBtConn());

#else // CONFIG_DOOR_LOCK_BLE_UWB

	return BleTypes::kInvalidProtocolVersion;

#endif // CONFIG_DOOR_LOCK_BLE_UWB
}

} // namespace Aliro
