/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <aliro_service/aliro_service.h>

#include "advertising.h"
#include "advertising_block_state.h"
#include "state.h"
#include "utils.h"

#include <aliro/aliro.h>
#include <aliro/ble_types.h>
#include <doorlock/utils/mutex_guard.h>
#include <doorlock/utils/utils.h>
#include <gatt_server/gatt_server.h>
#include <l2cap_server/l2cap_server.h>

#ifdef CONFIG_DOOR_LOCK_ALIRO_BLE_SERVICE_DYNAMIC_TAG
#include <time_utils/time_utils.h>
#endif // CONFIG_DOOR_LOCK_ALIRO_BLE_SERVICE_DYNAMIC_TAG

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <cerrno>
#include <cstring>
#include <tuple>

LOG_MODULE_REGISTER(AliroService, CONFIG_DOOR_LOCK_ALIRO_BLE_SERVICE_LOG_LEVEL);

namespace DoorLock::AliroService {

namespace {

using ::DoorLock::Utils::MutexGuard;

constexpr size_t kMaxSessions{ CONFIG_DOOR_LOCK_ALIRO_BLE_SERVICE_MAX_SESSIONS };

static_assert(CONFIG_BT_MAX_CONN >= CONFIG_DOOR_LOCK_ALIRO_BLE_SERVICE_MAX_SESSIONS,
	      "CONFIG_BT_MAX_CONN must be greater than or equal to the configured Aliro service session limit");

K_MUTEX_DEFINE(sMutex);

State sState;
size_t sConnectionCount;
AdvertisingState sAdvertisingBlockState;
bool sWasDisconnected;

void ConnectedCallback(bt_conn *conn, uint8_t error);
void DisconnectedCallback(bt_conn *conn, uint8_t reason);
void RecycledCallback();

BT_CONN_CB_DEFINE(sConnCallbacks){
	.connected = ConnectedCallback,
	.disconnected = DisconnectedCallback,
	.recycled = RecycledCallback,
};

void StartAdvertisingWorkHandler(k_work *);
K_WORK_DEFINE(sStartAdvertisingWork, StartAdvertisingWorkHandler);

void RestartAdvertisingWorkHandler(k_work *);
K_WORK_DEFINE(sRestartAdvertisingWork, RestartAdvertisingWorkHandler);

void UpdateAdvertisingDataWorkHandler(k_work *);
K_WORK_DELAYABLE_DEFINE(sUpdateAdvertisingDataWork, UpdateAdvertisingDataWorkHandler);

bool CanStartAdvertisingLocked()
{
	return sState.IsStarted() && !sAdvertisingBlockState.IsBlocked() && sConnectionCount < kMaxSessions;
}

int PrepareAdvertisingDataLocked()
{
	constexpr auto notification{ Aliro::BleTypes::AdvertisingServiceData::Notification::NoError };

	Aliro::BleTypes::BleExpiryTimestamp expiryTime{ Aliro::BleTypes::kExpiryTimeUnavailable };

#ifdef CONFIG_DOOR_LOCK_ALIRO_BLE_SERVICE_DYNAMIC_TAG
	const auto unixTime = TimeUtils::GetCurrentUnixTime();
	if (unixTime.has_value()) {
		expiryTime = unixTime.value() + CONFIG_DOOR_LOCK_ALIRO_BLE_SERVICE_DYNAMIC_TAG_EXPIRY_DURATION_S;
	}
#endif // CONFIG_DOOR_LOCK_ALIRO_BLE_SERVICE_DYNAMIC_TAG

	int err = Advertising::SetData(notification, expiryTime);
	VerifyOrReturnValue(err == 0, err);

#ifdef CONFIG_DOOR_LOCK_ALIRO_BLE_SERVICE_DYNAMIC_TAG
	if (unixTime.has_value()) {
		std::ignore =
			k_work_reschedule(&sUpdateAdvertisingDataWork,
					  K_SECONDS(CONFIG_DOOR_LOCK_ALIRO_BLE_SERVICE_DYNAMIC_TAG_EXPIRY_DURATION_S));
	}
#endif // CONFIG_DOOR_LOCK_ALIRO_BLE_SERVICE_DYNAMIC_TAG

	return 0;
}

bool IsAliroConnection(const bt_conn *conn)
{
	bt_conn_info info{};

	if (!conn || bt_conn_get_info(conn, &info) != 0) {
		return false;
	}

	return info.id == Utils::kAliroBtId;
}

void ConnectedCallback(bt_conn *conn, uint8_t error)
{
	VerifyOrReturn(IsAliroConnection(conn));
	VerifyOrReturn(conn && error == 0, LOG_ERR("Aliro BLE connection failed (conn: %p, error: %u)", conn, error));

	const auto *ref = bt_conn_ref(conn);
	VerifyOrReturn(ref, LOG_ERR("Failed to increment Aliro connection reference count"));

	[[maybe_unused]] size_t connectionCount{ 0 };
	bool shouldRestartAdvertising{ false };

	{
		MutexGuard lock{ sMutex };
		sConnectionCount++;
		connectionCount = sConnectionCount;
		shouldRestartAdvertising = CanStartAdvertisingLocked();
	}

	LOG_INF("Aliro BLE connected: %p, connections: %zu/%zu", conn, connectionCount, kMaxSessions);

	VerifyOrReturn(shouldRestartAdvertising);
	std::ignore = k_work_submit(&sRestartAdvertisingWork);
}

void DisconnectedCallback(bt_conn *conn, uint8_t reason)
{
	VerifyOrReturn(IsAliroConnection(conn));

	[[maybe_unused]] size_t connectionCount{ 0 };

	{
		MutexGuard lock{ sMutex };

		if (sConnectionCount > 0) {
			sConnectionCount--;
		}

		sWasDisconnected = true;
		connectionCount = sConnectionCount;
	}

	bt_conn_unref(conn);

	LOG_INF("Aliro BLE disconnected: %p, reason: %u, connections: %zu/%zu", conn, reason, connectionCount,
		kMaxSessions);
}

void RecycledCallback()
{
	bool shouldRestartAdvertising{ false };

	{
		MutexGuard lock{ sMutex };
		shouldRestartAdvertising = CanStartAdvertisingLocked() && sWasDisconnected;
	}

	VerifyOrReturn(shouldRestartAdvertising);
	std::ignore = k_work_submit(&sRestartAdvertisingWork);
}

int Disconnect(bt_conn *conn)
{
	const int err = bt_conn_disconnect(conn, BT_HCI_ERR_REMOTE_USER_TERM_CONN);
	VerifyOrReturnValue(err == 0 || err == -ENOTCONN, err,
			    LOG_ERR("Failed to disconnect Aliro connection: %d", err));
	return 0;
}

int DisconnectAll()
{
	{
		MutexGuard lock{ sMutex };
		VerifyOrReturnValue(sState.IsInitialized(), -ENODEV, LOG_ERR("Aliro service not initialized"));
	}

	int result{ 0 };

	bt_conn_foreach(
		BT_CONN_TYPE_LE,
		[](bt_conn *conn, void *userData) {
			if (!IsAliroConnection(conn)) {
				return;
			}

			auto *disconnectResult = static_cast<int *>(userData);
			const auto err = Disconnect(conn);
			if (err != 0 && *disconnectResult == 0) {
				*disconnectResult = err;
			}
		},
		&result);

	return result;
}

void StartAdvertisingWorkHandler(k_work *)
{
	MutexGuard lock{ sMutex };

	VerifyOrReturn(sState.IsStarted());
	VerifyOrReturn(PrepareAdvertisingDataLocked() == 0);
	VerifyOrReturn(CanStartAdvertisingLocked());

	sWasDisconnected = false;
	std::ignore = Advertising::Start();
}

void RestartAdvertisingWorkHandler(k_work *)
{
	MutexGuard lock{ sMutex };

	VerifyOrReturn(CanStartAdvertisingLocked());

	sWasDisconnected = false;
	std::ignore = Advertising::Start();
}

void UpdateAdvertisingDataWorkHandler(k_work *)
{
	MutexGuard lock{ sMutex };

	VerifyOrReturn(sState.IsStarted());
	std::ignore = PrepareAdvertisingDataLocked();
}

} // namespace

int Init()
{
	MutexGuard lock{ sMutex };

	VerifyOrReturnValue(!sState.IsInitialized(), -EALREADY, LOG_ERR("Aliro service already initialized"));

	Aliro::BleTypes::BleAddress address;
	int err = Utils::CreateBtId(address);
	VerifyOrReturnValue(err == 0, err, LOG_ERR("Failed to create Aliro Bluetooth ID: %d", err));

	err = bt_enable(nullptr);
	VerifyOrReturnValue(err == 0 || err == -EALREADY, err, LOG_ERR("Failed to enable BLE stack: %d", err));

	constexpr L2capServer::Callbacks l2capCallbacks{
		.mAccept =
			[](bt_conn *conn) {
				const auto protocolVersion = GattServer::GetBleUwbProtocolVersion(conn);
				return protocolVersion != Aliro::BleTypes::kInvalidProtocolVersion;
			},
		.mOnConnected =
			[](bt_conn *conn) {
				auto *ref = bt_conn_ref(conn);
				VerifyOrReturn(ref, LOG_ERR("Failed to increment Aliro connection reference count"));
				const auto error =
					Aliro::AliroStack::Instance().CreateSession(Aliro::ConnectionHandle::Ble(conn));
				VerifyOrReturn(error == ALIRO_NO_ERROR, bt_conn_unref(conn));
			},
		.mOnDisconnected =
			[](bt_conn *conn) {
				Aliro::AliroStack::Instance().DestroySession(Aliro::ConnectionHandle::Ble(conn));
			},
		.mOnDataReceived =
			[](bt_conn *conn, uint8_t *data, size_t length) {
				Aliro::AliroStack::Instance().HandleSessionData(Aliro::ConnectionHandle::Ble(conn),
										{ data, length });
			},
	};

	err = L2capServer::Init(l2capCallbacks);
	VerifyOrReturnValue(err == 0, err, LOG_ERR("Failed to initialize L2CAP server: %d", err));

	const GattServer::SupportedFeatures supportedFeatures{
		.TimesyncProcedure0 = static_cast<uint8_t>(
			IS_ENABLED(CONFIG_DOOR_LOCK_ALIRO_BLE_SERVICE_TIMESYNC_PROCEDURE_0) ? 1U : 0U),
		.TimesyncProcedure1 = static_cast<uint8_t>(
			IS_ENABLED(CONFIG_DOOR_LOCK_ALIRO_BLE_SERVICE_TIMESYNC_PROCEDURE_1) ? 1U : 0U),
		.LeCodedPhy = static_cast<uint8_t>(IS_ENABLED(CONFIG_BT_CTLR_PHY_CODED) ? 1U : 0U),
	};

	err = GattServer::Init(L2capServer::GetSpsm(), supportedFeatures);
	VerifyOrReturnValue(err == 0, err, LOG_ERR("Failed to initialize GATT server: %d", err));

	err = Advertising::Init(address);
	VerifyOrReturnValue(err == 0, err, LOG_ERR("Failed to initialize Aliro advertising: %d", err));

	sState.SetInitialized();
	return 0;
}

int Start()
{
	{
		MutexGuard lock{ sMutex };
		VerifyOrReturnValue(sState.IsInitialized(), -ENODEV, LOG_ERR("Aliro service not initialized"));
		VerifyOrReturnValue(!sState.IsStarted(), -EALREADY, LOG_ERR("Aliro service already started"));
	}

	int err = GattServer::Register();
	VerifyOrReturnValue(err == 0, err, LOG_ERR("Failed to register GATT server: %d", err));

	{
		MutexGuard lock{ sMutex };
		sState.SetStarted();
	}

	std::ignore = k_work_submit(&sStartAdvertisingWork);
	return 0;
}

int Stop()
{
	{
		MutexGuard lock{ sMutex };
		VerifyOrReturnValue(sState.IsInitialized(), -ENODEV, LOG_ERR("Aliro service not initialized"));
		VerifyOrReturnValue(sState.IsStarted(), -EALREADY, LOG_ERR("Aliro service not started"));

		sState.SetStopped();
		sWasDisconnected = false;
	}

	k_work_sync sync;
	std::ignore = k_work_cancel_sync(&sStartAdvertisingWork, &sync);
	std::ignore = k_work_cancel_sync(&sRestartAdvertisingWork, &sync);
	std::ignore = k_work_cancel_delayable_sync(&sUpdateAdvertisingDataWork, &sync);

	{
		MutexGuard lock{ sMutex };
		std::ignore = Advertising::Stop();
	}

	std::ignore = DisconnectAll();
	std::ignore = GattServer::Unregister();

	return 0;
}

void RefreshAdvertising()
{
	{
		MutexGuard lock{ sMutex };
		VerifyOrReturn(sState.IsInitialized(), LOG_WRN("Aliro service not initialized"));
	}

	std::ignore = k_work_reschedule(&sUpdateAdvertisingDataWork, K_NO_WAIT);
}

int BlockAdvertising(AdvertisingBlockReason reason)
{
	MutexGuard lock{ sMutex };
	VerifyOrReturnValue(sState.IsInitialized(), -ENODEV, LOG_ERR("Aliro service not initialized"));

	if (!sAdvertisingBlockState.Block(reason) || !sState.IsStarted()) {
		return 0;
	}

	return Advertising::Stop();
}

int UnblockAdvertising(AdvertisingBlockReason reason)
{
	bool shouldRestartAdvertising{ false };

	{
		MutexGuard lock{ sMutex };
		VerifyOrReturnValue(sState.IsInitialized(), -ENODEV, LOG_ERR("Aliro service not initialized"));

		if (!sAdvertisingBlockState.Unblock(reason)) {
			return 0;
		}

		shouldRestartAdvertising = CanStartAdvertisingLocked();
	}

	if (shouldRestartAdvertising) {
		std::ignore = k_work_submit(&sRestartAdvertisingWork);
	}

	return 0;
}

int Send(Aliro::ConnectionHandle handle, Aliro::Data data)
{
	{
		MutexGuard lock{ sMutex };
		VerifyOrReturnValue(sState.IsInitialized(), -ENODEV, LOG_ERR("Aliro service not initialized"));
	}

	VerifyOrReturnValue(handle.IsBle(), -EINVAL, LOG_ERR("Invalid BLE connection handle"));
	VerifyOrReturnValue(IsAliroConnection(handle.GetBtConn()), -EINVAL,
			    LOG_ERR("Connection does not belong to Aliro service"));

	return L2capServer::Send(handle.GetBtConn(), data.mData, data.mLength);
}

int Terminate(Aliro::ConnectionHandle handle)
{
	{
		MutexGuard lock{ sMutex };
		VerifyOrReturnValue(sState.IsInitialized(), -ENODEV, LOG_ERR("Aliro service not initialized"));
	}

	VerifyOrReturnValue(handle.IsBle(), -EINVAL, LOG_ERR("Invalid BLE connection handle"));

	auto *conn = handle.GetBtConn();
	VerifyOrReturnValue(IsAliroConnection(conn), -EINVAL, LOG_ERR("Connection does not belong to Aliro service"));

	const int err = Disconnect(conn);
	bt_conn_unref(conn);
	VerifyOrReturnValue(err == 0, err, LOG_ERR("Failed to disconnect Aliro connection: %d", err));

	return 0;
}

Aliro::ProtocolVersion GetProtocolVersion(Aliro::ConnectionHandle handle)
{
	VerifyOrReturnValue(handle.IsBle(), Aliro::BleTypes::kInvalidProtocolVersion,
			    LOG_ERR("Invalid BLE connection handle"));

	const bt_conn *conn = handle.GetBtConn();
	return GattServer::GetBleUwbProtocolVersion(conn);
}

} // namespace DoorLock::AliroService
