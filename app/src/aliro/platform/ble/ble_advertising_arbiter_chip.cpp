/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

/**
 * @file
 *   BLE Advertising Arbiter implementation for Matter builds.
 *
 * This implementation delegates to Matter's BLEAdvertisingArbiter to ensure
 * compatibility with Matter's BLE requirements. It is compiled only when
 * CONFIG_CHIP is enabled.
 */

#include "ble_advertising_arbiter.h"

#include <platform/CHIPDeviceLayer.h>
#include <platform/Zephyr/BLEAdvertisingArbiter.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/logging/log.h>

#include <array>
#include <iterator>

// Undefine Matter macros that conflict with Aliro utilities
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

#include "aliro/utils.h"

LOG_MODULE_REGISTER(BleAdvArbiter, CONFIG_DOOR_LOCK_BLE_LOG_LEVEL);

using namespace ::chip;
using namespace ::chip::DeviceLayer;

namespace DoorLock::Interface::BleAdvertisingArbiter {

namespace {

struct RequestSlot {
	Request *mRequest{ nullptr };
	chip::DeviceLayer::BLEAdvertisingArbiter::Request mMatterRequest{};
};

std::array<RequestSlot, kNumberOfComponents> sSlots{};

} // namespace

AliroError InsertRequest(Component component, Request &request)
{
	const auto idx = static_cast<size_t>(component);
	VerifyOrReturnStatus(idx < kNumberOfComponents, ALIRO_INVALID_ARGUMENT, LOG_ERR("Invalid component"));

	auto &slot = sSlots[idx];

	slot.mRequest = &request;

	// Map to Matter arbiter's request structure
	// Priority is based on Component enum order (lower index = higher priority)
	auto &matterReq = slot.mMatterRequest;
	matterReq.priority = static_cast<uint8_t>(idx);
	matterReq.options = request.mOptions;
	matterReq.minInterval = request.mMinInterval;
	matterReq.maxInterval = request.mMaxInterval;
	matterReq.advertisingData = Span<const bt_data>(request.mAdvertisingData, std::size(request.mAdvertisingData));
	matterReq.scanResponseData =
		Span<const bt_data>(request.mScanResponseData, std::size(request.mScanResponseData));
	matterReq.onStarted = nullptr;
	matterReq.onStopped = nullptr;

	// Forward to Matter's arbiter
	PlatformMgr().LockChipStack();
	CHIP_ERROR err = chip::DeviceLayer::BLEAdvertisingArbiter::InsertRequest(matterReq);
	PlatformMgr().UnlockChipStack();

	VerifyOrReturnStatus(err == CHIP_NO_ERROR, ALIRO_ERROR_INTERNAL,
			     LOG_ERR("Failed to insert advertising request for %s", ComponentToString(component)));

	LOG_INF("Inserted advertising request for %s", ComponentToString(component));

	return ALIRO_NO_ERROR;
}

AliroError CancelRequest(Component component)
{
	const auto idx = static_cast<size_t>(component);
	VerifyOrReturnStatus(idx < kNumberOfComponents, ALIRO_INVALID_ARGUMENT, LOG_ERR("Invalid component"));

	auto &slot = sSlots[idx];
	VerifyOrReturnStatus(slot.mRequest, ALIRO_NO_ERROR, LOG_ERR("No request to cancel"));

	// Forward to Matter's arbiter
	PlatformMgr().LockChipStack();
	chip::DeviceLayer::BLEAdvertisingArbiter::CancelRequest(slot.mMatterRequest);
	PlatformMgr().UnlockChipStack();

	slot.mRequest = nullptr;

	LOG_INF("Cancelled advertising request for %s", ComponentToString(component));

	return ALIRO_NO_ERROR;
}

} // namespace DoorLock::Interface::BleAdvertisingArbiter
