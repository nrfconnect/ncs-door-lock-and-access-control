/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/sys/slist.h>

#include <cstdint>

namespace DoorLock::Interface::BleAdvertisingArbiter {

/**
 * @file
 *   BLE Advertising Arbiter for the Door Lock application.
 *
 * This module coordinates BLE advertising between different application components.
 * When multiple advertising requests are submitted, the arbiter selects the one with
 * the highest priority based on the priority field and starts BLE advertising using
 * parameters defined in the winning request.
 */

/**
 * @brief BLE advertising request structure.
 */
struct Request : public sys_snode_t {
	uint8_t mPriority; ///< Advertising priority (lower value = higher priority)
	uint32_t mOptions; ///< Advertising options (BT_LE_ADV_OPT_XXX)
	uint16_t mMinInterval; ///< Minimum advertising interval (0.625 ms units)
	uint16_t mMaxInterval; ///< Maximum advertising interval (0.625 ms units)
	bt_data mAdvertisingData[2]; ///< Advertising data buffer
	bt_data mScanResponseData[1]; ///< Scan response data buffer
};

/**
 * @brief Request BLE advertising for a component.
 *
 * If the request has higher priority than other active requests, BLE
 * advertising is restarted using the new request's parameters.
 *
 * @note This function does not take ownership of the Request object, so the object
 *       must not get destroyed and shall remain valid after the call until the Request
 *       is cancelled by CancelRequest().
 *
 * @param request   The advertising request.
 * @return 0 on success, or a negative value on error.
 */
int InsertRequest(Request &request);

/**
 * @brief Cancel BLE advertising request for a component.
 *
 * If the cancelled request was the top-priority one, advertising is
 * restarted with the next highest-priority request, or stopped if
 * no other requests are active.
 *
 * @param request   The advertising request.
 * @return 0 on success, or a negative value on error.
 */
int CancelRequest(Request &request);

} // namespace DoorLock::Interface::BleAdvertisingArbiter
