/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include "aliro/errors.h"

#include <zephyr/bluetooth/bluetooth.h>

#include <cstdint>

namespace DoorLock::Interface::BleAdvertisingArbiter {

/**
 * @file
 *   BLE Advertising Arbiter for the Door Lock application.
 *
 * This module coordinates BLE advertising between different application components
 * (Aliro, SMP DFU, NUS). When multiple components request BLE advertising at the
 * same time, the arbiter selects the one with the highest priority based on the
 * Component enum order (Aliro > Smp > Nus) and starts BLE advertising using
 * parameters defined in the winning request.
 *
 * Two implementations exist:
 * - Non-Matter (ble_advertising_arbiter.cpp): Directly manages bt_le_adv_* calls
 * - Matter (ble_advertising_arbiter_chip.cpp): Delegates to Matter's BLEAdvertisingArbiter
 *
 * The appropriate implementation is selected at compile time based on CONFIG_CHIP.
 */

/**
 * @brief BLE advertising component identifiers.
 *
 * Components are ordered by default priority (lower value = higher priority).
 */
enum class Component : uint8_t {
	Aliro, ///< Aliro BLE/UWB service
	Smp, ///< SMP DFU service
	Nus, ///< Nordic UART Service
	None ///< None
};

/**
 * @brief Number of components.
 */
static constexpr size_t kNumberOfComponents{ static_cast<size_t>(Component::None) };

/**
 * @brief Convert component enum to string for logging.
 */
constexpr const char *ComponentToString(Component component)
{
	switch (component) {
	case Component::Aliro:
		return "Aliro";
	case Component::Smp:
		return "SMP";
	case Component::Nus:
		return "NUS";
	case Component::None:
		return "None";
	default:
		return "Unknown";
	}
}

/**
 * @brief BLE advertising request structure.
 */
struct Request {
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
 * @param component The component making the request.
 * @param request   The advertising request parameters.
 * @return ALIRO_NO_ERROR on success, error code otherwise.
 */
AliroError InsertRequest(Component component, Request &request);

/**
 * @brief Cancel BLE advertising request for a component.
 *
 * If the cancelled request was the top-priority one, advertising is
 * restarted with the next highest-priority request, or stopped if
 * no other requests are active.
 *
 * @param component The component cancelling its request.
 */
AliroError CancelRequest(Component component);

} // namespace DoorLock::Interface::BleAdvertisingArbiter
