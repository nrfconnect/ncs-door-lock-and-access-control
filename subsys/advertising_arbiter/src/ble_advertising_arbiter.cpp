/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

/**
 * @file
 *   BLE Advertising Arbiter implementation for non-Matter builds.
 *
 * This implementation directly manages BLE advertising using Zephyr's bt_le_adv_* APIs.
 * It is compiled only when CONFIG_CHIP is not enabled.
 */

#include <advertising_arbiter/ble_advertising_arbiter.h>

#include <doorlock/utils/mutex_guard.h>
#include <doorlock/utils/utils.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <array>
#include <cstring>
#include <iterator>

LOG_MODULE_REGISTER(BleAdvArbiter, CONFIG_DOOR_LOCK_ADVERTISING_ARBITER_LOG_LEVEL);

namespace DoorLock::Interface::BleAdvertisingArbiter {

namespace {

using DoorLock::Utils::MutexGuard;

K_MUTEX_DEFINE(sMutex);
sys_slist_t sRequests{};
bool sWasDisconnected{ false };
bool sRetry{ false };

const Request *GetTopPriorityRequest()
{
	const auto *head = sys_slist_peek_head(&sRequests);
	return static_cast<const Request *>(head);
}

void AddRequest(Request &request)
{
	sys_snode_t *prev{ nullptr };

	// Find the previous request
	{
		sys_snode_t *node{ nullptr };
		SYS_SLIST_FOR_EACH_NODE (&sRequests, node) {
			const auto &nodeReq = *static_cast<const Request *>(node);
			if (request.mPriority < nodeReq.mPriority) {
				break;
			}
			prev = node;
		}
	}

	// Insert the request into the list
	if (!prev) {
		sys_slist_prepend(&sRequests, &request);
	} else {
		sys_slist_insert(&sRequests, prev, &request);
	}
}

int RemoveRequest(Request &request)
{
	const auto removed = sys_slist_find_and_remove(&sRequests, &request);
	VerifyOrReturnValue(removed, -ENOENT, LOG_ERR("Request not found"));
	return 0;
}

size_t GetConnectionCount()
{
	size_t count = 0;

	bt_conn_foreach(
		BT_CONN_TYPE_ALL,
		[](bt_conn *conn, void *data) {
			bt_conn_info info{};
			VerifyOrReturn(bt_conn_get_info(conn, &info) == 0);
			VerifyOrReturn(info.id == BT_ID_DEFAULT);
			VerifyOrReturn(info.state == BT_CONN_STATE_CONNECTED);
			auto &count = *static_cast<size_t *>(data);
			++count;
		},
		&count);

	return count;
}

int StartAdvertising(const Request &request)
{
	sRetry = true;

	if ((request.mOptions & BT_LE_ADV_OPT_CONN) &&
	    GetConnectionCount() >= CONFIG_DOOR_LOCK_ADVERTISING_ARBITER_MAX_CONNECTIONS) {
		LOG_WRN("Cannot start connectable advertising: max connections reached");
		return -ENOMEM;
	}

	bt_le_adv_param advParam =
		BT_LE_ADV_PARAM_INIT(request.mOptions, request.mMinInterval, request.mMaxInterval, nullptr);

	int err = bt_le_adv_start(&advParam, request.mAdvertisingData, std::size(request.mAdvertisingData),
				  request.mScanResponseData, std::size(request.mScanResponseData));

	VerifyOrReturnValue(err == 0, err, LOG_ERR("Failed to start advertising for request %p: %d", &request, err));

	LOG_INF("Started advertising for request %p", &request);

	sRetry = false;
	return 0;
}

int StopAdvertising(const Request &request)
{
	sRetry = false;

	const int err = bt_le_adv_stop();
	VerifyOrReturnValue(err == 0, err, LOG_ERR("Failed to stop advertising: %d", err));

	LOG_INF("Stopped advertising for request %p", &request);

	return 0;
}

bool IsDefaultIdentity(bt_conn *conn)
{
	VerifyOrReturnValue(conn, false);

	bt_conn_info info{};
	const int err = bt_conn_get_info(conn, &info);
	VerifyOrReturnValue(err == 0, false, LOG_ERR("bt_conn_get_info failed (%d)", err));

	return info.id == BT_ID_DEFAULT;
}

BT_CONN_CB_DEFINE(sConnCallbacks){
	.disconnected =
		[](bt_conn *conn, uint8_t) {
			VerifyOrReturn(IsDefaultIdentity(conn));
			sWasDisconnected = true;
		},
	.recycled =
		[]() {
			MutexGuard lock{ sMutex };

			VerifyOrReturn(sWasDisconnected || sRetry);
			sWasDisconnected = false;

			const auto *request = GetTopPriorityRequest();
			VerifyOrReturn(request, LOG_INF("No advertising requests pending"));

			const int err = StartAdvertising(*request);
			VerifyOrReturn(err == 0,
				       LOG_ERR("Failed to restart advertising for request %p: %d", request, err));
		},
};

} // namespace

int InsertRequest(Request &request)
{
	MutexGuard lock{ sMutex };

	// Get the previous top-priority request
	const auto *prevRequest = GetTopPriorityRequest();

	// Insert new request into the list
	AddRequest(request);

	// Get the current top-priority request, return if there is no change
	const auto *currentRequest = GetTopPriorityRequest();
	VerifyOrReturnValue(currentRequest != prevRequest, 0, LOG_DBG("No change in advertising request"));

	// Stop advertising for the previous request
	if (prevRequest) {
		const int err = StopAdvertising(*prevRequest);
		VerifyOrReturnValue(err == 0, err);
	}

	// Start advertising for the current request
	return StartAdvertising(*currentRequest);
}

int CancelRequest(Request &request)
{
	MutexGuard lock{ sMutex };

	// If the cancelled request is currently active, stop advertising
	const auto *currentRequest = GetTopPriorityRequest();
	VerifyOrReturnValue(currentRequest, -ENOENT, LOG_ERR("Advertising request list empty"));

	if (currentRequest == &request) {
		const int err = StopAdvertising(*currentRequest);
		VerifyOrReturnValue(err == 0, err);
	}

	// Remove the cancelled request from the list
	const int err = RemoveRequest(request);
	VerifyOrReturnValue(err == 0, err);

	// Start advertising for the next highest priority request, if any
	const auto *nextRequest = GetTopPriorityRequest();
	VerifyOrReturnValue(nextRequest, 0, LOG_INF("No more advertising requests pending"));

	return StartAdvertising(*nextRequest);
}

} // namespace DoorLock::Interface::BleAdvertisingArbiter
