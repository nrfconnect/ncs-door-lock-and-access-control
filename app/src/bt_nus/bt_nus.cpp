/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "bt_nus.h"

#include "aliro/utils.h"

#include "aliro/platform/ble/ble_advertising_arbiter.h"

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/logging/log.h>
#include <zephyr/settings/settings.h>

#include <cstring>

LOG_MODULE_REGISTER(NusService, CONFIG_DOOR_LOCK_APP_LOG_LEVEL);

namespace Aliro::BtNus {

namespace BleArbiter = DoorLock::Interface::BleAdvertisingArbiter;

AliroError NUSService::Start()
{
	VerifyOrReturnStatus(!mIsStarted, ALIRO_INVALID_STATE, LOG_ERR("NUS service is already started"));

	static bt_conn_auth_cb sConnAuthCallbacks = {
		.passkey_display = [](bt_conn *conn,
				      unsigned int passkey) { Instance().AuthPasskeyDisplay(conn, passkey); },
		.cancel = [](bt_conn *conn) { Instance().AuthCancel(conn); },
#if defined(CONFIG_BT_APP_PASSKEY)
		.app_passkey =
			[](bt_conn *) {
				const uint32_t passkey{ CONFIG_DOOR_LOCK_BT_NUS_APP_PASSKEY };
				return passkey;
			},
#endif // CONFIG_BT_APP_PASSKEY
	};

	static bt_conn_auth_info_cb sConnAuthInfoCallbacks = {
		.pairing_complete = [](bt_conn *conn, bool bonded) { Instance().PairingComplete(conn, bonded); },
		.pairing_failed = [](bt_conn *conn, bt_security_err reason) { Instance().PairingFailed(conn, reason); },
	};

	static bt_nus_cb sNusCallbacks = {
		.received = [](bt_conn *conn, const uint8_t *data,
			       uint16_t len) { Instance().RxCallback(conn, data, len); },
	};

	VerifyOrReturnStatus(bt_conn_auth_cb_register(&sConnAuthCallbacks) == 0, ALIRO_ERROR_INTERNAL,
			     LOG_ERR("Failed to register auth callbacks"));
	VerifyOrReturnStatus(bt_conn_auth_info_cb_register(&sConnAuthInfoCallbacks) == 0, ALIRO_ERROR_INTERNAL,
			     LOG_ERR("Failed to register auth info callbacks"));
	VerifyOrReturnStatus(bt_nus_init(&sNusCallbacks) == 0, ALIRO_ERROR_INTERNAL,
			     LOG_ERR("Failed to initialize NUS service"));

	mRequest = BleArbiter::Request{ .mOptions = BT_LE_ADV_OPT_CONN,
					.mMinInterval = BT_GAP_ADV_FAST_INT_MIN_2,
					.mMaxInterval = BT_GAP_ADV_FAST_INT_MAX_2 };

	// Set advertising data buffers
	mRequest.mAdvertisingData[0] = BT_DATA(BT_DATA_FLAGS, &kAdvertisingFlags, sizeof(kAdvertisingFlags));
	mRequest.mAdvertisingData[1] = BT_DATA(BT_DATA_UUID128_ALL, kNusUuid.data(), kNusUuid.size());

	const char *deviceName = bt_get_name();
	mRequest.mScanResponseData[0] =
		BT_DATA(BT_DATA_NAME_COMPLETE, deviceName, static_cast<uint8_t>(strlen(deviceName)));

	AliroError err = BleArbiter::InsertRequest(BleArbiter::Component::Nus, mRequest);
	VerifyOrReturnStatus(err == ALIRO_NO_ERROR, err,
			     LOG_ERR("NUS advertising request failed (rc %d)", err.ToInt()));

	mIsStarted = true;
	LOG_INF("NUS service started");
	return ALIRO_NO_ERROR;
}

void NUSService::StopServer()
{
	VerifyOrReturn(IsNusStarted(), LOG_ERR("NUS service not started"));

	BleArbiter::CancelRequest(BleArbiter::Component::Nus);

	mIsStarted = false;
	LOG_INF("NUS service stopped");
}

bool NUSService::RegisterCommand(const char *const name, size_t length, CommandCallback callback, void *context)
{
	VerifyOrReturnFalse(name, LOG_ERR("Command name is null"));
	VerifyOrReturnFalse(mCommandsCounter < CONFIG_DOOR_LOCK_BT_NUS_MAX_COMMANDS,
			    LOG_ERR("Too many commands registered"));
	VerifyOrReturnFalse(length <= CONFIG_DOOR_LOCK_BT_NUS_MAX_COMMAND_LEN, LOG_ERR("Command name too long"));

	mCommandsList[mCommandsCounter++] = Command(name, length, callback, context);

	return true;
}

void NUSService::DispatchCommand(const char *const data, uint16_t len)
{
	VerifyOrReturn(len > 0, LOG_DBG("Empty command received, ignoring"));

	static constexpr char LF = '\n';
	static constexpr char CR = '\r';
	static const char *CRLF = "\r\n";

	auto isCommandMatch = [data, len](const Command &c) {
		if (len < c.mCommandLength) {
			return false;
		}

		if (memcmp(data, c.mCommand.data(), c.mCommandLength) != 0) {
			return false;
		}

		if (len == c.mCommandLength) {
			return true;
		}

		// Check if the remaining part is a valid endline
		const char *endline = data + c.mCommandLength;
		const size_t endlineLen = len - c.mCommandLength;

		// Single character endlines (\n or \r)
		if (endlineLen == 1) {
			return (*endline == LF || *endline == CR);
		}

		// Windows endline (\r\n)
		if (endlineLen == 2) {
			return memcmp(endline, CRLF, 2) == 0;
		}

		return false;
	};

	for (uint8_t i = 0; i < mCommandsCounter; ++i) {
		const auto &c = mCommandsList[i];
		if (isCommandMatch(c)) {
			if (c.mCallback) {
				c.mCallback(c.mContext);
			}
			return;
		}
	}
	LOG_ERR("NUS command unknown!");
}

bool NUSService::SendData(const char *const data, size_t length)
{
	VerifyOrReturnFalse(IsNusStarted(), LOG_ERR("NUS service not started"));
	VerifyOrReturnFalse(mBTConnection, LOG_ERR("No BLE connection"));
	VerifyOrReturnFalse(bt_conn_get_security(mBTConnection) >= BT_SECURITY_L2,
			    LOG_ERR("Security requirements not met"));
	VerifyOrReturnFalse(bt_nus_send(mBTConnection, reinterpret_cast<const uint8_t *const>(data), length) == 0,
			    LOG_ERR("Failed to send NUS data"));

	return true;
}

void NUSService::Connected(bt_conn *conn, uint8_t err)
{
	VerifyOrReturn(IsNusStarted(), LOG_DBG("NUS service not started, ignoring connection"));
	VerifyOrReturn(!err, LOG_ERR("NUS Connection failed (err %u)", err));
	VerifyOrReturn(conn, LOG_ERR("Connection is null"));

	mBTConnection = conn;
	bt_conn_set_security(conn, BT_SECURITY_L3);
	LOG_INF("NUS connected");
}

void NUSService::Disconnected(bt_conn *, uint8_t reason)
{
	VerifyOrReturn(IsNusStarted(), LOG_DBG("NUS service not started, ignoring disconnection"));

	mBTConnection = nullptr;
	LOG_INF("NUS disconnected (reason: %u)", reason);
}

void NUSService::SecurityChanged(bt_conn *conn, bt_security_t level, bt_security_err err)
{
	VerifyOrReturn(IsNusStarted(), LOG_DBG("NUS service not started, ignoring security change"));

	VerifyOrReturn(err == BT_SECURITY_ERR_SUCCESS, LOG_ERR("NUS BT Security failed: level %d err %d",
							       static_cast<int>(level), static_cast<int>(err)));
	LOG_DBG("NUS BT Security changed: %s level %d", GetAddressString(conn), static_cast<int>(level));
}

void NUSService::RxCallback(bt_conn *, const uint8_t *const data, uint16_t len)
{
	VerifyOrReturn(bt_conn_get_security(mBTConnection) >= BT_SECURITY_L2,
		       LOG_ERR("Received NUS command, but security requirements are not met"));

	LOG_DBG("NUS received: %d bytes", len);
	DispatchCommand(reinterpret_cast<const char *>(data), len);
}

void NUSService::AuthPasskeyDisplay(bt_conn *, unsigned int passkey)
{
	LOG_INF("PROVIDE THE FOLLOWING CODE IN YOUR MOBILE APP: %d", passkey);
}

void NUSService::AuthCancel(bt_conn *conn)
{
	LOG_INF("NUS BT Pairing cancelled: %s", GetAddressString(conn));
	bt_conn_disconnect(conn, BT_HCI_ERR_REMOTE_USER_TERM_CONN);
}

void NUSService::PairingComplete(bt_conn *conn, bool bonded)
{
	VerifyOrReturn(IsNusStarted(), LOG_DBG("NUS service not started, ignoring pairing complete"));
	LOG_DBG("NUS BT Pairing completed: %s, bonded: %d", GetAddressString(conn), bonded);
}

void NUSService::PairingFailed(bt_conn *conn, bt_security_err reason)
{
	VerifyOrReturn(IsNusStarted(), LOG_DBG("NUS service not started, ignoring pairing failed"));
	LOG_ERR("NUS BT Pairing failed to %s : reason %d", GetAddressString(conn), static_cast<uint8_t>(reason));
}

char *NUSService::GetAddressString(bt_conn *conn)
{
#if CONFIG_LOG
	static char addr[BT_ADDR_LE_STR_LEN];
	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));
	return addr;
#else
	return nullptr;
#endif
}

} // namespace Aliro::BtNus
