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

#include "ble_advertising_arbiter.h"

#include "aliro/utils.h"
#include "mutex_guard.h"

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/logging/log.h>

#include <array>
#include <cstring>
#include <iterator>
#include <utility>

LOG_MODULE_REGISTER(BleAdvArbiter, CONFIG_DOOR_LOCK_BLE_LOG_LEVEL);

namespace DoorLock::Interface::BleAdvertisingArbiter {

namespace {

K_MUTEX_DEFINE(sMutex);

// Registered requests per component
std::array<Request *, kNumberOfComponents> sRequests{};

// Currently active (advertising) request
Request *sActiveRequest{ nullptr };

// Find the highest priority request based on Component enum order (lower index = higher priority)
std::pair<Request *, Component> FindTopPriorityRequest()
{
	for (size_t idx = 0; idx < kNumberOfComponents; ++idx) {
		if (sRequests[idx]) {
			return { sRequests[idx], static_cast<Component>(idx) };
		}
	}

	return { nullptr, Component::None };
}

// Get the component that is currently advertising
Component GetActiveComponent()
{
	for (size_t idx = 0; idx < kNumberOfComponents; ++idx) {
		if (sRequests[idx] == sActiveRequest) {
			return static_cast<Component>(idx);
		}
	}
	return Component::None;
}

AliroError StopAdvertising()
{
	VerifyOrReturnStatus(sActiveRequest, ALIRO_NO_ERROR, LOG_INF("Currently not advertising"));

	int err = bt_le_adv_stop();
	VerifyOrReturnStatus(err == 0, AliroError::FromInt(err), LOG_ERR("Failed to stop advertising: %d", err));

	LOG_INF("Stopped advertising for %s", ComponentToString(GetActiveComponent()));

	sActiveRequest = nullptr;

	return ALIRO_NO_ERROR;
}

size_t GetConnectionCount()
{
	size_t count = 0;

	bt_conn_foreach(
		BT_CONN_TYPE_ALL,
		[](bt_conn *conn, void *data) {
			bt_conn_info info{};
			if (bt_conn_get_info(conn, &info) == 0 && info.state == BT_CONN_STATE_CONNECTED) {
				(*static_cast<size_t *>(data))++;
			}
		},
		&count);

	return count;
}

AliroError StartAdvertising(Component component, Request &request)
{
	if ((request.mOptions & BT_LE_ADV_OPT_CONN) && GetConnectionCount() >= CONFIG_BT_MAX_CONN) {
		LOG_WRN("Cannot start connectable advertising: max connections reached");
		return ALIRO_NO_MEMORY;
	}

	bt_le_adv_param advParam =
		BT_LE_ADV_PARAM_INIT(request.mOptions, request.mMinInterval, request.mMaxInterval, nullptr);

	int err = bt_le_adv_start(&advParam, request.mAdvertisingData, std::size(request.mAdvertisingData),
				  request.mScanResponseData, std::size(request.mScanResponseData));

	VerifyOrReturnStatus(err == 0, AliroError::FromInt(err),
			     LOG_ERR("Failed to start advertising for %s: %d", ComponentToString(component), err));

	sActiveRequest = &request;

	LOG_INF("Started advertising for %s", ComponentToString(component));

	return ALIRO_NO_ERROR;
}

} // namespace

AliroError InsertRequest(Component component, Request &request)
{
	MutexGuard lock{ sMutex };

	const auto componentIdx = static_cast<size_t>(component);
	sRequests[componentIdx] = &request;

	auto [top, topComponent] = FindTopPriorityRequest();
	VerifyOrReturnStatus(top, ALIRO_ERROR_INTERNAL, LOG_ERR("Failed to find top priority request"));

	// Try to update advertising data first.
	// If advertising is not active, bt_le_adv_update_data returns -EAGAIN and we start advertising.
	int err = bt_le_adv_update_data(top->mAdvertisingData, std::size(top->mAdvertisingData), top->mScanResponseData,
					std::size(top->mScanResponseData));

	if (err == 0) {
		sActiveRequest = top;
		LOG_INF("Advertising data updated successfully for %s", ComponentToString(topComponent));
		return ALIRO_NO_ERROR;
	} else if (err == -EAGAIN) {
		return StartAdvertising(topComponent, *top);
	}

	LOG_ERR("Failed to update advertising data for %s: %d", ComponentToString(topComponent), err);

	return AliroError::FromInt(err);
}

AliroError CancelRequest(Component component)
{
	MutexGuard lock{ sMutex };

	const auto componentIdx = static_cast<size_t>(component);
	Request *requestToCancel = sRequests[componentIdx];

	// If the cancelled request is currently active, stop advertising
	if (sActiveRequest == requestToCancel) {
		ReturnErrorOnFailure(StopAdvertising());
	}

	sRequests[componentIdx] = nullptr;

	// Start advertising for the next highest priority request, if any
	auto [newTop, newTopComponent] = FindTopPriorityRequest();
	if (!newTop) {
		LOG_INF("No more advertising requests pending");
		return ALIRO_NO_ERROR;
	}

	return StartAdvertising(newTopComponent, *newTop);
}

} // namespace DoorLock::Interface::BleAdvertisingArbiter
