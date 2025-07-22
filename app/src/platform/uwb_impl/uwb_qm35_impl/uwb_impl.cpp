/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "uwb_impl.h"

#include "aliro/utils.h"

// UWB API
#include <aliro_uwb_adapter/aliro_uwb_adapter.h>
#include <aliro_uwb_adapter/aliro_uwb_session.h>
#include <qsemaphore.h>

// Calibration data
#include "calibration/qm35825_calib.h"

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(uwb, CONFIG_NCS_DOOR_LOCK_APP_LOG_LEVEL);

namespace {

K_EVENT_DEFINE(sUwbEvents);

constexpr uint32_t kUwbWaitCapabilitiesTimeoutMs{ 1000 };

/**
 * @brief Helper enum class for UWB events.
 */
enum class UwbEvents : uint32_t {
	Timeout = 0x00,
	Error = 0x01,
	DeviceCaps = 0x02,
};

} // namespace

namespace Aliro::Uwb {

static void UwbCallback(cherry_core_event *event, void *user_data)
{
	if (event->type == CHERRY_CORE_EVENT_TYPE_ERROR) {
		k_event_set(&sUwbEvents, ToUnderlying(UwbEvents::DeviceCaps));
	} else if (event->type == CHERRY_CORE_EVENT_TYPE_DEVICE_CAPS) {
		auto *caps = event->data.device_caps;
		if (caps->status_err == CHERRY_ERR_NONE && caps->ccc_capabilities) {
			k_event_set(&sUwbEvents, ToUnderlying(UwbEvents::DeviceCaps));
		}
	}

	// Free allocated event.
	cherry_core_event_free(event);
}

AliroError UltraWideBandImpl::_Init([[maybe_unused]] Callbacks callbacks)
{
	cherry_err cErr{};
	uint32_t event{};

	LOG_INF("Initializing UWB device...");

	mCtx = cherry_create("qm35", &UwbCallback, nullptr);
	VerifyOrReturnStatus(mCtx, ALIRO_UWB_INIT_FAILED, LOG_ERR("Failed to create Cherry context"));

	// Use full calibration data for the QM35825 device.
	// This needs to be done only once during initialization, or after a power cycle.
	const auto calibData = &util_calib_qm35825;
	int err = cherry_set_calib(mCtx, calibData);
	VerifyOrExit(err == CHERRY_ERR_NONE, LOG_ERR("Failed to set calibration data: %d", err));

	// Initialize Cherry device capabilities.
	cErr = cherry_get_device_capabilities(mCtx);
	VerifyOrExit(cErr == CHERRY_ERR_NONE, LOG_ERR("Failed to get device capabilities: %s", cherry_err_str(cErr)));

	// Wait for device capabilities or error to be received.
	event = k_event_wait(&sUwbEvents, 0xFFFF, false, K_MSEC(kUwbWaitCapabilitiesTimeoutMs));
	VerifyAndExit(event == ToUnderlying(UwbEvents::Timeout), LOG_ERR("Timeout, capabilities not received."));
	VerifyAndExit(event == ToUnderlying(UwbEvents::Error),
		      LOG_ERR("Error occurred while waiting for device capabilities."));
	VerifyOrExit(event == ToUnderlying(UwbEvents::DeviceCaps),
		     LOG_ERR("Unexpected event while waiting for device capabilities (%d).", event));

	LOG_INF("CCC capabilities available and UWB device initialized successfully.");

	return ALIRO_NO_ERROR;

exit:
	_Deinit();

	return ALIRO_UWB_INIT_FAILED;
}

AliroError UltraWideBandImpl::_Deinit()
{
	if (mCtx) {
		cherry_destroy_sync(mCtx);
		mCtx = nullptr;
	}

	LOG_INF("UWB device deinitialized successfully.");

	return ALIRO_NO_ERROR;
}

void UltraWideBandImpl::_BleTimeSync() const
{
	LOG_INF("Start Bluetooth LE and UWB time synchronization, procedure 0");
}

AliroError UltraWideBandImpl::_HandleBleMessage(const uint8_t *data, size_t length) const
{
	VerifyOrReturnStatus(mCtx, ALIRO_INVALID_STATE, LOG_ERR("UWB is not initialized."));
	VerifyOrReturnStatus(data && length > 0, ALIRO_INVALID_ARGUMENT, LOG_ERR("Invalid BLE message data."));

	return ALIRO_ERROR_NOT_IMPLEMENTED;
}

AliroError UltraWideBandImpl::_ConfigureRangingSession(const uint8_t *ursk, size_t urskLen)
{
	VerifyOrReturnStatus(mCtx, ALIRO_INVALID_STATE, LOG_ERR("UWB is not initialized."));
	VerifyOrReturnStatus(ursk && urskLen > 0, ALIRO_INVALID_ARGUMENT, LOG_ERR("Invalid URSK."));

	return ALIRO_ERROR_NOT_IMPLEMENTED;
}

AliroError UltraWideBandImpl::_InitateRangingSession()
{
	VerifyOrReturnStatus(mCtx, ALIRO_INVALID_STATE, LOG_ERR("UWB is not initialized."));

	return ALIRO_ERROR_NOT_IMPLEMENTED;
}

AliroError UltraWideBandImpl::_TerminateRangingSession()
{
	VerifyOrReturnStatus(mCtx, ALIRO_INVALID_STATE, LOG_ERR("UWB is not initialized."));

	return ALIRO_ERROR_NOT_IMPLEMENTED;
}

AliroError UltraWideBandImpl::_SuspendRangingSession()
{
	VerifyOrReturnStatus(mCtx, ALIRO_INVALID_STATE, LOG_ERR("UWB is not initialized."));

	return ALIRO_ERROR_NOT_IMPLEMENTED;
}

AliroError UltraWideBandImpl::_ResumeRangingSession()
{
	VerifyOrReturnStatus(mCtx, ALIRO_INVALID_STATE, LOG_ERR("UWB is not initialized."));

	return ALIRO_ERROR_NOT_IMPLEMENTED;
}

} // namespace Aliro::Uwb
