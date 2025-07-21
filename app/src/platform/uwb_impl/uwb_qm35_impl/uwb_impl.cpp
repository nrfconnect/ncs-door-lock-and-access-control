/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "uwb_impl.h"
#include "uwb_message.h"

#include "aliro/utils.h"

// UWB API
#include <aliro_uwb_adapter/aliro_uwb_adapter.h>
#include <aliro_uwb_adapter/aliro_uwb_session.h>

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

/**
 * @brief Converts aliro_uwb_err to AliroError.
 *
 * @param uwbErr The UWB error code to convert.
 *
 * @return The corresponding AliroError.
 */
constexpr AliroError ConvertUwbError(aliro_uwb_err uwbErr)
{
	switch (uwbErr) {
	case ALIRO_UWB_ERR_NONE:
		return ALIRO_NO_ERROR;
	case ALIRO_UWB_ERR_INVALID_PARAMETER:
	case ALIRO_UWB_ERR_MSG_MALFORMED:
		return ALIRO_INVALID_ARGUMENT;
	case ALIRO_UWB_ERR_UWBS_TIMEOUT:
		return ALIRO_TIMEOUT;
	case ALIRO_UWB_ERR_INTERNAL:
	case ALIRO_UWB_ERR_SESSION_INIT:
		return ALIRO_ERROR_INTERNAL;
	case ALIRO_UWB_ERR_SESSION_ACTIVE:
	case ALIRO_UWB_ERR_SESSION_CONFIG:
	case ALIRO_UWB_ERR_MESSAGE_STATE:
	case ALIRO_UWB_ERR_INVALID_STATE:
		return ALIRO_INVALID_STATE;
	default:
		return ALIRO_ERROR_INTERNAL;
	}
}

#ifdef CONFIG_ALIRO_UWB_SESSION_LOGGING

/**
 * @brief Converts Cherry CCC session state to readable string.
 *
 * @param state The Cherry CCC session state value.
 *
 * @return String representation of the session state.
 */
constexpr const char *CherryCccSessionStateToString(enum cherry_ccc_session_state state)
{
	switch (state) {
	case CHERRY_CCC_SESSION_STATE_INIT:
		return "INIT";
	case CHERRY_CCC_SESSION_STATE_DEINIT:
		return "DEINIT";
	case CHERRY_CCC_SESSION_STATE_ACTIVE:
		return "ACTIVE";
	case CHERRY_CCC_SESSION_STATE_IDLE:
		return "IDLE";
	default:
		return "UNKNOWN";
	}
}

/**
 * @brief Converts Cherry CCC session reason code to readable string.
 *
 * @param reasonCode The Cherry CCC session reason code value.
 *
 * @return String representation of the reason code.
 */
constexpr const char *CherryCccReasonCodeToString(enum cherry_ccc_state_change_reason reasonCode)
{
	switch (reasonCode) {
	case CHERRY_CCC_STATE_CHANGE_REASON_MGMT_CMD:
		return "MGMT_CMD";
	case CHERRY_CCC_STATE_CHANGE_REASON_UNKNOWN:
		return "UNKNOWN_REASON";
	case CHERRY_CCC_STATE_CHANGE_REASON_FORCE_STOPPED:
		return "FORCE_STOPPED";
	default:
		return "UNKNOWN";
	}
}

#endif // CONFIG_ALIRO_UWB_SESSION_LOGGING

} // namespace

namespace Aliro::Uwb {

void UwbCoreCallback(cherry_core_event *event, void *userData)
{
	VerifyOrExit(event && userData, LOG_ERR("Invalid event or user data."));

	{
		auto *uwbImpl = static_cast<UltraWideBandImpl *>(userData);

		if (event->type == CHERRY_CORE_EVENT_TYPE_ERROR) {
			k_event_set(&sUwbEvents, ToUnderlying(UwbEvents::Error));
		} else if (event->type == CHERRY_CORE_EVENT_TYPE_DEVICE_CAPS) {
			auto *caps = event->data.device_caps;
			if (caps->status_err == CHERRY_ERR_NONE && caps->ccc_capabilities) {
				uwbImpl->mCoreEvent = event;
				k_event_set(&sUwbEvents, ToUnderlying(UwbEvents::DeviceCaps));
				return;
			}
		}
	}

exit:
	// Free allocated event.
	cherry_core_event_free(event);
}

void SessionHandlerCallback(aliro_uwb_session_event *event, void *user_data)
{
	auto sessionData = event->data;
	switch (event->type) {
	case ALIRO_UWB_SESSION_EVENT_TYPE_SESSION_STATUS:
#ifdef CONFIG_ALIRO_UWB_SESSION_LOGGING

		LOG_INF("Session status changed: %s (%d), reason: %s (%d)",
			CherryCccSessionStateToString(sessionData.status->session_state),
			sessionData.status->session_state, CherryCccReasonCodeToString(sessionData.status->reason_code),
			sessionData.status->reason_code);

#else // CONFIG_ALIRO_UWB_SESSION_LOGGING

		LOG_INF("Session status changed: %d, reason: %d", sessionData.status->session_state,
			sessionData.status->reason_code);

#endif // CONFIG_ALIRO_UWB_SESSION_LOGGING
		break;
	case ALIRO_UWB_SESSION_EVENT_TYPE_SESSION_ERROR:
		LOG_INF("Session error: 0x%x", sessionData.error->status_err);
		break;
	case ALIRO_UWB_SESSION_EVENT_TYPE_SESSION_CONTROLLER_REPORT: {
		const cherry_ccc_controller_session_report *results = sessionData.controller_report;
		LOG_INF("Controller report %d measurements", results->n_measurements);
		break;
	}
	case ALIRO_UWB_SESSION_EVENT_TYPE_SESSION_CONTROLEE_REPORT: {
		const cherry_ccc_session_controlee_measurements *currentMeasurement =
			sessionData.controlee_report->measurements;
		while (currentMeasurement) {
			if (!currentMeasurement->frame_status) {
				LOG_INF("Controlee report distance %d [cm]", currentMeasurement->distance_cm);
			} else {
				LOG_INF("Controlee report error status: 0x%x on slot id: %d",
					currentMeasurement->frame_status, currentMeasurement->slot_index);
			}
			currentMeasurement = currentMeasurement->next;
		}
		break;
	}
	default:
		break;
	}

	// Free the consumed event.
	aliro_uwb_session_event_free(event);
}

void TransmitBleMessage(aliro_uwb_message *message, aliro_uwb_session *sessionCtx, void *userData, bool timeout)
{
	auto *uwbImpl = static_cast<UltraWideBandImpl *>(userData);

	uwbImpl->mCallbacks.mTransmitBleMessage(uwbImpl->mAliroSessionUserData, message->data, message->len);

	// Free the consumed message.
	aliro_uwb_session_message_free(message);
}

AliroError UltraWideBandImpl::_Init(const Callbacks &callbacks)
{
	cherry_err cErr{};
	uint32_t event{};

	mCallbacks = callbacks;

	LOG_INF("Initializing UWB device...");

	mCtx = cherry_create("qm35", &UwbCoreCallback, this);
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

	LOG_DBG("CCC capabilities available.");
	mAliroCtx = aliro_uwb_adapter_create_reader(mCtx, mCoreEvent->data.device_caps, &mReaderConfig);
	cherry_core_event_free(mCoreEvent);
	mCoreEvent = nullptr;

	VerifyOrExit(mAliroCtx, LOG_ERR("Failed to create UWB adapter reader."));

	LOG_INF("UWB device initialized successfully.");

	return ALIRO_NO_ERROR;

exit:
	_Deinit();

	return ALIRO_UWB_INIT_FAILED;
}

AliroError UltraWideBandImpl::_Deinit()
{
	_TerminateRangingSession();

	if (mAliroCtx) {
		aliro_uwb_adapter_destroy(mAliroCtx);
		mAliroCtx = nullptr;
	}

	// The Cherry context is destroyed last, after all other resources.
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

AliroError UltraWideBandImpl::_HandleBleMessage(const uint8_t *data, size_t length)
{
	VerifyOrReturnStatus(data && length > 0, ALIRO_INVALID_ARGUMENT, LOG_ERR("Invalid BLE message data."));

	aliro_uwb_err err{};
	Message message(length);
	VerifyOrExit(message, LOG_ERR("Memory allocation failed."));

	memcpy(message->data, data, length);
	message->len = length;

	err = aliro_uwb_session_message_handle(mAliroSessionCtx, message.get());
	VerifyOrReturnStatus(err == ALIRO_UWB_ERR_NONE, ConvertUwbError(err),
			     LOG_ERR("Cannot handle UWB session message 0x%x", err));

	return ALIRO_NO_ERROR;

exit:
	_TerminateRangingSession();

	return ALIRO_ERROR_INTERNAL;
}

AliroError UltraWideBandImpl::_ConfigureRangingSession(SessionIdentifier sessionId, const CryptoTypes::Ursk &ursk,
						       void *sessionUserData)
{
	VerifyOrReturnStatus(mAliroCtx, ALIRO_INVALID_STATE, LOG_ERR("UWB is not initialized."));

	aliro_uwb_err uwbErr{ ALIRO_UWB_ERR_INTERNAL };

	mAliroSessionUserData = sessionUserData;
	mAliroSessionCtx =
		aliro_uwb_session_create(mAliroCtx, sessionId, &SessionHandlerCallback, &TransmitBleMessage, this);
	VerifyOrExit(mAliroSessionCtx, LOG_ERR("Failed to create UWB session."));

	uwbErr = aliro_uwb_session_set_ursk(mAliroSessionCtx, ursk.data());
	VerifyOrExit(uwbErr == ALIRO_UWB_ERR_NONE, LOG_ERR("Failed to set URSK in UWB session: 0x%x", uwbErr));

	return ALIRO_NO_ERROR;

exit:
	_TerminateRangingSession();

	return ConvertUwbError(uwbErr);
}

AliroError UltraWideBandImpl::_InitiateRangingSession()
{
	VerifyOrReturnStatus(mAliroSessionCtx, ALIRO_INVALID_STATE, LOG_ERR("UWB session is not initialized."));
	aliro_uwb_err err = aliro_uwb_session_init_setup(mAliroSessionCtx);
	VerifyOrReturnStatus(err == ALIRO_UWB_ERR_NONE, ConvertUwbError(err),
			     LOG_ERR("Failed to initialize UWB session setup: 0x%x", err));

	return ALIRO_NO_ERROR;
}

AliroError UltraWideBandImpl::_TerminateRangingSession()
{
	if (mAliroSessionCtx) {
		aliro_uwb_session_destroy(mAliroSessionCtx);
		mAliroSessionCtx = nullptr;
		mAliroSessionUserData = nullptr;
	}

	return ALIRO_NO_ERROR;
}

AliroError UltraWideBandImpl::_SuspendRangingSession()
{
	return ALIRO_ERROR_NOT_IMPLEMENTED;
}

AliroError UltraWideBandImpl::_ResumeRangingSession()
{
	return ALIRO_ERROR_NOT_IMPLEMENTED;
}

} // namespace Aliro::Uwb
