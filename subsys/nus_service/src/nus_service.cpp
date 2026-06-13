/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <nus_service/nus_service.h>

#include "advertising.h"

#include <doorlock/utils/utils.h>

#include <bluetooth/services/nus.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/logging/log.h>

#include <algorithm>
#include <array>
#include <cstring>

LOG_MODULE_REGISTER(NusService, CONFIG_DOOR_LOCK_NUS_SERVICE_LOG_LEVEL);

namespace DoorLock::NUSService {

namespace {

using CommandCallback = DoorLock::NUSService::CommandCallback;

struct Command {
	std::array<uint8_t, CONFIG_DOOR_LOCK_NUS_SERVICE_MAX_COMMAND_LEN> mCommand;
	size_t mCommandLength;
	CommandCallback mCallback;
	void *mContext;

	Command() = default;

	Command(const char *name, size_t length, CommandCallback callback, void *context)
		: mCommandLength(length), mCallback(callback), mContext(context)
	{
		std::copy_n(reinterpret_cast<const uint8_t *>(name), length, mCommand.begin());
	}
};

bool sIsStarted;
std::array<Command, CONFIG_DOOR_LOCK_NUS_SERVICE_MAX_COMMANDS> sCommandsList{};
uint8_t sCommandsCounter;
bt_conn *sBTConnection;

void Connected(bt_conn *conn, uint8_t error);
void Disconnected(bt_conn *conn, uint8_t reason);
void RxCallback(bt_conn *conn, const uint8_t *data, uint16_t len);

BT_CONN_CB_DEFINE(doorlock_nus_conn_callbacks) = {
	.connected = Connected,
	.disconnected = Disconnected,
};

bt_nus_cb sNusCallbacks = {
	.received = RxCallback,
};

bool IsDefaultBtId(const bt_conn *conn)
{
	bt_conn_info info{};
	if (bt_conn_get_info(conn, &info) != 0) {
		return false;
	}
	return info.id == BT_ID_DEFAULT;
}

bool IsNusStarted()
{
	return sIsStarted;
}

void DispatchCommand(const char *const data, uint16_t len)
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

		const char *endline = data + c.mCommandLength;
		const size_t endlineLen = len - c.mCommandLength;

		if (endlineLen == 1) {
			return (*endline == LF || *endline == CR);
		}

		if (endlineLen == 2) {
			return memcmp(endline, CRLF, 2) == 0;
		}

		return false;
	};

	for (uint8_t i = 0; i < sCommandsCounter; ++i) {
		const auto &c = sCommandsList[i];
		if (isCommandMatch(c)) {
			if (c.mCallback) {
				c.mCallback(c.mContext);
			}
			return;
		}
	}
	LOG_ERR("NUS command unknown!");
}

void RxCallback(bt_conn *conn, const uint8_t *const data, uint16_t len)
{
	VerifyOrReturn(sBTConnection == conn, LOG_ERR("NUS connection not established"));
	VerifyOrReturn(bt_conn_get_security(conn) >= BT_SECURITY_L2,
		       LOG_ERR("Received NUS command, but security requirements are not met"));

	LOG_DBG("NUS received: %d bytes", len);
	DispatchCommand(reinterpret_cast<const char *>(data), len);
}

void Connected(bt_conn *conn, uint8_t err)
{
	VerifyOrReturn(conn, LOG_ERR("Connection is null"));
	VerifyOrReturn(!err, LOG_ERR("NUS Connection failed (err %u)", err));
	VerifyOrReturn(IsDefaultBtId(conn));
	VerifyOrReturn(IsNusStarted(), LOG_DBG("NUS service not started, ignoring connection"));
	VerifyOrReturn(Advertising::IsStarted(), LOG_DBG("NUS advertising not started, ignoring connection"));
	VerifyOrReturn(sBTConnection == nullptr, LOG_WRN("NUS connection already established"));

	sBTConnection = bt_conn_ref(conn);
	VerifyOrReturn(sBTConnection, LOG_ERR("Failed to increment NUS connection reference count"));

	bt_conn_set_security(sBTConnection, BT_SECURITY_L3);
	LOG_INF("NUS connected");
}

void Disconnected(bt_conn *conn, uint8_t reason)
{
	VerifyOrReturn(IsDefaultBtId(conn));
	VerifyOrReturn(IsNusStarted(), LOG_DBG("NUS service not started, ignoring disconnection"));
	VerifyOrReturn(sBTConnection == conn);

	bt_conn_unref(sBTConnection);
	sBTConnection = nullptr;

	LOG_INF("NUS disconnected (reason: %u)", reason);
}

} // anonymous namespace

int Init()
{
	const int err = bt_nus_init(&sNusCallbacks);
	VerifyOrReturnValue(err == 0, err, LOG_ERR("Failed to initialize NUS service: %d", err));

	return 0;
}

int Start(uint8_t priority, uint16_t minInterval, uint16_t maxInterval)
{
	VerifyOrReturnValue(!IsNusStarted(), -EALREADY, LOG_ERR("NUS service is already started"));

	const int err = Advertising::InsertRequest(priority, minInterval, maxInterval);
	VerifyOrReturnValue(err == 0, err, LOG_ERR("NUS advertising request failed: %d", err));

	sIsStarted = true;
	LOG_INF("NUS service started");
	return 0;
}

void Stop()
{
	VerifyOrReturn(IsNusStarted(), LOG_ERR("NUS service not started"));

	sIsStarted = false;

	Advertising::CancelRequest();

	if (sBTConnection) {
		const int err = bt_conn_disconnect(sBTConnection, BT_HCI_ERR_REMOTE_USER_TERM_CONN);
		if (err != 0) {
			LOG_ERR("Failed to disconnect NUS connection: %d", err);
		}
		bt_conn_unref(sBTConnection);
		sBTConnection = nullptr;
	}

	LOG_INF("NUS service stopped");
}

int RegisterCommand(const char *const name, size_t length, CommandCallback callback, void *context)
{
	VerifyOrReturnValue(name, -EINVAL, LOG_ERR("Command name is null"));
	VerifyOrReturnValue(IN_RANGE(length, 1, CONFIG_DOOR_LOCK_NUS_SERVICE_MAX_COMMAND_LEN), -EINVAL,
			    LOG_ERR("Command name too long"));
	VerifyOrReturnValue(sCommandsCounter < CONFIG_DOOR_LOCK_NUS_SERVICE_MAX_COMMANDS, -ENOMEM,
			    LOG_ERR("Too many commands registered"));

	sCommandsList[sCommandsCounter++] = Command(name, length, callback, context);

	return 0;
}

int Send(const char *const data, size_t length)
{
	VerifyOrReturnValue(IsNusStarted(), -ENODEV, LOG_ERR("NUS service not started"));
	VerifyOrReturnValue(sBTConnection, -ENOTCONN, LOG_ERR("No BLE connection"));

	auto *ref = bt_conn_ref(sBTConnection);
	VerifyOrReturnValue(ref, -EIO, LOG_ERR("Failed to increment NUS connection reference count"));

	int status{ -EIO };

	VerifyOrExit(bt_conn_get_security(ref) >= BT_SECURITY_L2, LOG_ERR("Security requirements not met"));

	status = bt_nus_send(ref, reinterpret_cast<const uint8_t *const>(data), length);
	VerifyOrExit(status == 0, LOG_ERR("Failed to send NUS data"));

exit:
	bt_conn_unref(ref);
	return status;
}

} // namespace DoorLock::NUSService
