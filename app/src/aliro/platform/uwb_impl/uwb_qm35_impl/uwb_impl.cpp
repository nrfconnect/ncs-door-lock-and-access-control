/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "uwb_impl.h"
#include "uwb_message.h"

#include "aliro/memory.h"
#include "aliro/mutex_guard.h"
#include "aliro/utils.h"

// UWB API
#include <aliro_uwb_adapter/aliro_uwb_adapter.h>
#include <aliro_uwb_adapter/aliro_uwb_session.h>

// Calibration data
#include "calibration/qm35825_calib.h"

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/byteorder.h>

LOG_MODULE_REGISTER(UwbImpl, CONFIG_NCS_ALIRO_UWB_LOG_LEVEL);

namespace {

using RangingSessionState = Aliro::RangingSessionState;

K_EVENT_DEFINE(sUwbEvents);

constexpr uint32_t kUwbWaitForEventsTimeoutMs{ 1000 };

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
constexpr const char *CherryCccSessionStateToString(uint32_t state)
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
constexpr const char *CherryCccReasonCodeToString(cherry_ccc_state_change_reason reasonCode)
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

constexpr const char *RangingSessionStateToString(RangingSessionState state)
{
	switch (state) {
	case RangingSessionState::Uninitialized:
		return "Uninitialized";
	case RangingSessionState::Initialized:
		return "Initialized";
	case RangingSessionState::Idle:
		return "Idle";
	case RangingSessionState::Ranging:
		return "Ranging";
	case RangingSessionState::RangingSuspended:
		return "RangingSuspended";
	case RangingSessionState::RangingResumed:
		return "RangingResumed";
	case RangingSessionState::Destroyed:
		return "Destroyed";
	default:
		return "Unknown";
	}
}

#endif // CONFIG_ALIRO_UWB_SESSION_LOGGING

} // namespace

namespace Aliro::Uwb {

void UltraWideBandImpl::UwbCoreCallback(cherry_core_event *event, void *userData)
{
	using DeviceCaps = cherry_core_event_device_capabilities;
	using DeviceInfo = cherry_core_event_device_info;

	VerifyOrExit(event && userData, LOG_ERR("Invalid event or user data."));

	{
		auto *uwbImpl = static_cast<UltraWideBandImpl *>(userData);

		switch (event->type) {
		case CHERRY_CORE_EVENT_TYPE_ERROR:
			k_event_set(&sUwbEvents, UwbEvents::Error);
			break;
		case CHERRY_CORE_EVENT_TYPE_DEVICE_CAPS: {
			const DeviceCaps *caps = event->data.device_caps;
			if (caps->status_err == CHERRY_ERR_NONE && caps->ccc_capabilities) {
				uwbImpl->mCoreEvent = event;
				k_event_set(&sUwbEvents, UwbEvents::DeviceCaps);
				return;
			}
			break;
		}
		case CHERRY_CORE_EVENT_TYPE_DEVICE_INFO: {
			const DeviceInfo *info = event->data.device_info;
			if (info->status_err == CHERRY_ERR_NONE && info->fw_version) {
				uwbImpl->mCoreEvent = event;
				k_event_set(&sUwbEvents, UwbEvents::DeviceInfo);
				return;
			}
			break;
		}
		default:
			break;
		}
	}

exit:
	// Free allocated event.
	cherry_core_event_free(event);
}

void UltraWideBandImpl::TransmitBleMessage(aliro_uwb_message *message, UwbSessionContext uwbSessionCtx, void *userData,
					   [[maybe_unused]] bool)
{
	VerifyOrExit(message && userData, LOG_ERR("UWB message or user data is null."));

	{
		auto *uwbImpl = static_cast<UltraWideBandImpl *>(userData);
		auto *sessionCtx = uwbImpl->FindSession(uwbSessionCtx);
		VerifyOrExit(sessionCtx, LOG_ERR("Session context not found"));

		VerifyAndCall(uwbImpl->mCallbacks.mTransmitBleMessage, sessionCtx->mSessionContextData, message->data,
			      message->len);
	}

exit:
	// Free the consumed message.
	aliro_uwb_session_message_free(message);
}

void UltraWideBandImpl::SessionHandlerCallback(aliro_uwb_session_event *event, void *userData)
{
	VerifyOrExit(event && userData, LOG_ERR("UWB session event or user data is null."));

	{
		const auto sessionData = event->data;
		auto *uwbImpl = static_cast<UltraWideBandImpl *>(userData);
		auto *sessionCtx = uwbImpl->FindSession(event->session);
		VerifyOrExit(sessionCtx, LOG_WRN("Session context not found"));

		switch (event->type) {
		case ALIRO_UWB_SESSION_EVENT_TYPE_SESSION_STATUS: {
			const auto *status = sessionData.status;
			const auto oldState = sessionCtx->mSessionState;
			const auto newState = status->session_state;
			const auto reason = status->reason_code;

			switch (oldState) {
			case CHERRY_CCC_SESSION_STATE_INIT:
				if (newState == CHERRY_CCC_SESSION_STATE_INIT) {
					sessionCtx->mRangingSessionState = RangingSessionState::Initialized;
				} else if (newState == CHERRY_CCC_SESSION_STATE_IDLE) {
					sessionCtx->mRangingSessionState = RangingSessionState::Idle;
				}
				break;
			case CHERRY_CCC_SESSION_STATE_IDLE:
				if (newState == CHERRY_CCC_SESSION_STATE_ACTIVE) {
					if (sessionCtx->mRangingSessionState == RangingSessionState::Idle) {
						sessionCtx->mRangingSessionState = RangingSessionState::Ranging;
					} else if (sessionCtx->mRangingSessionState ==
						   RangingSessionState::RangingSuspended) {
						sessionCtx->mRangingSessionState = RangingSessionState::RangingResumed;
					}
				} else if (newState == CHERRY_CCC_SESSION_STATE_DEINIT) {
					sessionCtx->mRangingSessionState = RangingSessionState::Destroyed;
				}
				break;
			case CHERRY_CCC_SESSION_STATE_ACTIVE:
				if (newState == CHERRY_CCC_SESSION_STATE_IDLE) {
					sessionCtx->mRangingSessionState = RangingSessionState::RangingSuspended;
				}
				break;
			default:
				LOG_ERR("Unknown old state: %u, new state: %u", static_cast<uint32_t>(oldState),
					static_cast<uint32_t>(newState));
				// Should not happen, but if it does, set the state to uninitialized.
				sessionCtx->mRangingSessionState = RangingSessionState::Uninitialized;
			}

#ifdef CONFIG_ALIRO_UWB_SESSION_LOGGING

			LOG_INF("Session status changed: %s (%u) | %s (%u) -> %s (%u) [reason: %s (%u)]",
				RangingSessionStateToString(sessionCtx->mRangingSessionState),
				static_cast<uint32_t>(sessionCtx->mRangingSessionState),
				CherryCccSessionStateToString(oldState), static_cast<uint32_t>(oldState),
				CherryCccSessionStateToString(newState), static_cast<uint32_t>(newState),
				CherryCccReasonCodeToString(reason), static_cast<uint32_t>(reason));

#else // CONFIG_ALIRO_UWB_SESSION_LOGGING

			LOG_DBG("Session status changed: %u -> %u [reason: %u]", static_cast<uint32_t>(oldState),
				static_cast<uint32_t>(newState), static_cast<uint32_t>(reason));

#endif // CONFIG_ALIRO_UWB_SESSION_LOGGING

			sessionCtx->mSessionState = newState;
			VerifyAndCall(uwbImpl->mCallbacks.mRangingSessionStateChanged, sessionCtx->mSessionContextData,
				      sessionCtx->mRangingSessionState);
			break;
		}
		case ALIRO_UWB_SESSION_EVENT_TYPE_SESSION_ERROR:
			LOG_INF("Session error: 0x%x", static_cast<uint32_t>(sessionData.error->status_err));
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

					sys_put_be16(currentMeasurement->distance_cm,
						     uwbImpl->mCurrentDistanceCm.data());
					VerifyAndCall(uwbImpl->mCallbacks.mRangingData, sessionCtx->mSessionContextData,
						      UwbRangingData{ .mData = uwbImpl->mCurrentDistanceCm.data(),
								      .mLength = uwbImpl->mCurrentDistanceCm.size() });
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
	}

exit:
	// Free the consumed event.
	aliro_uwb_session_event_free(event);
}

AliroError UltraWideBandImpl::HandleDeviceCapsEvent(CoreEvent *event)
{
	VerifyOrReturnStatus(event && event->data.device_caps, ALIRO_INVALID_ARGUMENT,
			     LOG_ERR("Invalid event object (device capabilities)."));
	mAliroCtx = aliro_uwb_adapter_create_reader(mCtx, event->data.device_caps, &mReaderConfig);
	VerifyOrReturnStatus(mAliroCtx, ALIRO_ERROR_INTERNAL, LOG_ERR("Failed to create UWB adapter reader."));

	return ALIRO_NO_ERROR;
}

AliroError UltraWideBandImpl::HandleDeviceInfoEvent(CoreEvent *event)
{
	VerifyOrReturnStatus(event && event->data.device_info, ALIRO_INVALID_ARGUMENT,
			     LOG_ERR("Invalid event object (device info)."));
	const char *fwVersion = event->data.device_info->fw_version;
	const size_t fwVersionLen = std::strlen(fwVersion) + 1;

	mQm35FirmwareVersion = Aliro::make_unique_array_nothrow<char>(fwVersionLen);
	VerifyOrReturnStatus(mQm35FirmwareVersion, ALIRO_NO_MEMORY,
			     LOG_ERR("Memory allocation failed for QM35 firmware version."));

	std::strcpy(mQm35FirmwareVersion.get(), fwVersion);
	LOG_INF("QM35 FW revision: %s", mQm35FirmwareVersion.get());

	return ALIRO_NO_ERROR;
}

AliroError UltraWideBandImpl::HandleUwbEvent(UwbEvents expectedEvent, EventHandler eventHandler)
{
	// Wait for the expected event or error to be received.
	uint32_t event = k_event_wait(&sUwbEvents, UwbEvents::All, false, K_MSEC(kUwbWaitForEventsTimeoutMs));

	AliroError result{ ALIRO_ERROR_INTERNAL };
	// Check for timeout
	VerifyOrExit(event != UwbEvents::Timeout,
		     LOG_ERR("Timeout waiting for UWB event (expected: 0x%x).", expectedEvent);
		     result = ALIRO_TIMEOUT;);

	// Check for error event
	VerifyOrExit(event != UwbEvents::Error,
		     LOG_ERR("Error occurred while waiting for UWB event (expected: 0x%x).", expectedEvent));

	// Check if we received the expected event
	VerifyOrExit(event == expectedEvent,
		     LOG_ERR("Unexpected UWB event (received: 0x%x, expected: 0x%x).", event, expectedEvent));

	LOG_DBG("UWB event received (0x%x).", event);

	// Call the event handler
	result = eventHandler(mCoreEvent);

exit:
	// Clean up: free the event and clear flags
	cherry_core_event_free(mCoreEvent);
	k_event_clear(&sUwbEvents, UwbEvents::All);
	mCoreEvent = nullptr;

	return result;
}

AliroError UltraWideBandImpl::_Init(const Callbacks &callbacks)
{
	cherry_err cErr{};
	AliroError err{};

	mCallbacks = callbacks;

	VerifyOrReturnStatus(k_mutex_init(&mMutex) == 0, ALIRO_ERROR_INTERNAL, LOG_ERR("Failed to initialize mutex"));
	sys_slist_init(&mActiveSessionsList);

	LOG_INF("Initializing UWB device...");

	mCtx = cherry_create("qm35", &UwbCoreCallback, this);
	VerifyOrReturnStatus(mCtx, ALIRO_UWB_INIT_FAILED, LOG_ERR("Failed to create Cherry context"));

	// Use full calibration data for the QM35825 device.
	// This needs to be done only once during initialization, or after a power cycle.
	const auto calibData = &util_calib_qm35825;
	int status = cherry_set_calib(mCtx, calibData);
	VerifyOrExit(status == CHERRY_ERR_NONE, LOG_ERR("Failed to set calibration data: %d", status));

	// Read Cherry device capabilities.
	cErr = cherry_get_device_capabilities(mCtx);
	VerifyOrExit(cErr == CHERRY_ERR_NONE, LOG_ERR("Failed to get device capabilities: %s", cherry_err_str(cErr)));

	// Wait for and handle device capabilities event.
	err = HandleUwbEvent(UwbEvents::DeviceCaps,
			     [](CoreEvent *event) -> AliroError { return Instance().HandleDeviceCapsEvent(event); });
	VerifyOrExit(err == ALIRO_NO_ERROR);

	// Read Cherry device info.
	cErr = cherry_get_device_info(mCtx);
	VerifyOrExit(cErr == CHERRY_ERR_NONE, LOG_ERR("Failed to get device info: %s", cherry_err_str(cErr)));

	// Wait for and handle device info event.
	err = HandleUwbEvent(UwbEvents::DeviceInfo,
			     [](CoreEvent *event) -> AliroError { return Instance().HandleDeviceInfoEvent(event); });
	VerifyOrExit(err == ALIRO_NO_ERROR);

	LOG_INF("UWB device initialized successfully.");

	return ALIRO_NO_ERROR;

exit:
	_Deinit();

	return ALIRO_UWB_INIT_FAILED;
}

AliroError UltraWideBandImpl::_Deinit()
{
	RemoveAllSessions();

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

void UltraWideBandImpl::_BleTimeSync()
{
	LOG_INF("Start Bluetooth LE and UWB time synchronization, procedure 0");
}

AliroError UltraWideBandImpl::_HandleBleMessage(const uint8_t *data, size_t length,
						SessionContextHandle sessionContextData)
{
	VerifyOrReturnStatus(sessionContextData, ALIRO_INVALID_ARGUMENT, LOG_ERR("Session context data is null."));
	VerifyOrReturnStatus(data && length > 0, ALIRO_INVALID_ARGUMENT, LOG_ERR("Invalid BLE message data."));

	aliro_uwb_err err{};
	Message message(length);
	VerifyOrReturnStatus(message, ALIRO_NO_MEMORY, LOG_ERR("Memory allocation failed."));

	memcpy(message->data, data, length);
	message->len = length;

	const auto *sessionCtx = FindSession(sessionContextData);
	VerifyOrReturnStatus(sessionCtx, ALIRO_SESSION_NOT_FOUND,
			     LOG_ERR("Session context not found for handle: %p", sessionContextData));
	err = aliro_uwb_session_message_handle(sessionCtx->mUwbSessionContext, message.get());
	VerifyOrReturnStatus(err == ALIRO_UWB_ERR_NONE, ConvertUwbError(err),
			     LOG_ERR("Cannot handle UWB session message 0x%x", ToUnderlying(err)));

	return ALIRO_NO_ERROR;
}

AliroError UltraWideBandImpl::_ConfigureRangingSession(SessionIdentifier sessionId, const CryptoTypes::Ursk &ursk,
						       SessionContextHandle sessionContextData)
{
	VerifyOrReturnStatus(mAliroCtx, ALIRO_INVALID_STATE, LOG_ERR("UWB is not initialized."));
	VerifyOrReturnStatus(sessionContextData, ALIRO_INVALID_ARGUMENT, LOG_ERR("Session context data is null."));

	auto *sessionCtx = FindSession(sessionContextData);
	VerifyOrReturnStatus(!sessionCtx, ALIRO_INVALID_STATE,
			     LOG_ERR("Session context already exists for handle: %p", sessionContextData));

	aliro_uwb_session *newSessionCtx =
		aliro_uwb_session_create(mAliroCtx, sessionId, &SessionHandlerCallback, &TransmitBleMessage, this);
	VerifyOrReturnStatus(newSessionCtx, ALIRO_NO_MEMORY, LOG_ERR("Failed to create UWB session."));

	AliroError err = ConvertUwbError(aliro_uwb_session_set_ursk(newSessionCtx, ursk.data()));
	VerifyOrExit(err == ALIRO_NO_ERROR, LOG_ERR("Failed to set URSK in UWB session: %d", err.ToInt()));

	err = AddSession({ .mUwbSessionContext = newSessionCtx, .mSessionContextData = sessionContextData });
	VerifyOrExit(err == ALIRO_NO_ERROR,
		     LOG_ERR("Failed to add session to the active sessions list: %d", err.ToInt()));

	LOG_INF("UWB session created with sessionContextData: %p", sessionContextData);
	return ALIRO_NO_ERROR;

exit:
	aliro_uwb_session_destroy(newSessionCtx);

	return err;
}

AliroError UltraWideBandImpl::_InitiateRangingSession(SessionContextHandle sessionContextData)
{
	VerifyOrReturnStatus(sessionContextData, ALIRO_INVALID_ARGUMENT, LOG_ERR("Session context data is null."));

	const auto *sessionCtx = FindSession(sessionContextData);
	VerifyOrReturnStatus(sessionCtx, ALIRO_SESSION_NOT_FOUND,
			     LOG_ERR("Session context not found for handle: %p", sessionContextData));

	aliro_uwb_err err = aliro_uwb_session_init_setup(sessionCtx->mUwbSessionContext);
	VerifyOrReturnStatus(err == ALIRO_UWB_ERR_NONE, ConvertUwbError(err),
			     LOG_ERR("Failed to initialize UWB session setup: 0x%x", ToUnderlying(err)));

	return ALIRO_NO_ERROR;
}

AliroError UltraWideBandImpl::_TerminateRangingSession(SessionContextHandle sessionContextData)
{
	VerifyOrReturnStatus(sessionContextData, ALIRO_INVALID_ARGUMENT, LOG_ERR("Session context data is null."));

	auto *sessionCtx = FindSession(sessionContextData);
	VerifyOrReturnStatus(sessionCtx, ALIRO_SESSION_NOT_FOUND,
			     LOG_ERR("Session context not found for handle: %p", sessionContextData));

	RemoveSession(sessionCtx);

	LOG_DBG("Terminating UWB session with context: %p", sessionContextData);

	return ALIRO_NO_ERROR;
}

AliroError UltraWideBandImpl::_SuspendRangingSession(SessionContextHandle sessionContextData)
{
	VerifyOrReturnStatus(sessionContextData, ALIRO_INVALID_ARGUMENT, LOG_ERR("Session context data is null."));
	const auto *sessionCtx = FindSession(sessionContextData);
	VerifyOrReturnStatus(sessionCtx, ALIRO_SESSION_NOT_FOUND,
			     LOG_ERR("Session context not found for handle: %p", sessionContextData));

	aliro_uwb_err err = aliro_uwb_session_suspend(sessionCtx->mUwbSessionContext);
	VerifyOrReturnStatus(err == ALIRO_UWB_ERR_NONE, ConvertUwbError(err),
			     LOG_ERR("Failed to suspend UWB session: 0x%x", ToUnderlying(err)));

	return ALIRO_NO_ERROR;
}

AliroError UltraWideBandImpl::_ResumeRangingSession(SessionContextHandle sessionContextData)
{
	VerifyOrReturnStatus(sessionContextData, ALIRO_INVALID_ARGUMENT, LOG_ERR("Session context data is null."));
	const auto *sessionCtx = FindSession(sessionContextData);
	VerifyOrReturnStatus(sessionCtx, ALIRO_SESSION_NOT_FOUND,
			     LOG_ERR("Session context not found for handle: %p", sessionContextData));

	aliro_uwb_err err = aliro_uwb_session_resume(sessionCtx->mUwbSessionContext);
	VerifyOrReturnStatus(err == ALIRO_UWB_ERR_NONE, ConvertUwbError(err),
			     LOG_ERR("Failed to resume UWB session: 0x%x", ToUnderlying(err)));

	return ALIRO_NO_ERROR;
}

AliroError UltraWideBandImpl::AddSession(const SessionContext &sessionCtx)
{
	auto newCtx = Aliro::new_nothrow<SessionContext>();
	VerifyOrReturnStatus(newCtx, ALIRO_NO_MEMORY, LOG_ERR("Memory allocation failed for session context."));

	newCtx->mUwbSessionContext = sessionCtx.mUwbSessionContext;
	newCtx->mSessionContextData = sessionCtx.mSessionContextData;

	MutexGuard lock{ mMutex };

	sys_slist_append(&mActiveSessionsList, &newCtx->mSessionContextNode);

	return ALIRO_NO_ERROR;
}

void UltraWideBandImpl::RemoveSession(SessionContext *sessionCtx)
{
	{
		MutexGuard lock{ mMutex };
		VerifyOrReturn(sys_slist_find_and_remove(&mActiveSessionsList, &sessionCtx->mSessionContextNode),
			       LOG_WRN("Session doesn't exist"));
	}

	DestroySession(sessionCtx);
	delete sessionCtx;
}

void UltraWideBandImpl::RemoveAllSessions()
{
	while (true) {
		SessionContext *sessionCtx = nullptr;

		{
			MutexGuard lock{ mMutex };
			sys_snode_t *node = sys_slist_get(&mActiveSessionsList);
			VerifyOrReturn(node);
			sessionCtx = CONTAINER_OF(node, SessionContext, mSessionContextNode);
		}

		DestroySession(sessionCtx);
		delete sessionCtx;
	}
}

UltraWideBandImpl::SessionContext *UltraWideBandImpl::FindSession(const UwbSessionContext uwbSessionCtx)
{
	SessionContext *sessionCtx{};

	MutexGuard lock{ mMutex };
	SYS_SLIST_FOR_EACH_CONTAINER (&mActiveSessionsList, sessionCtx, mSessionContextNode) {
		if (sessionCtx->mUwbSessionContext == uwbSessionCtx) {
			return sessionCtx;
		}
	}

	return nullptr;
}

UltraWideBandImpl::SessionContext *UltraWideBandImpl::FindSession(SessionContextHandle sessionContextData)
{
	SessionContext *sessionCtx{};

	MutexGuard lock{ mMutex };
	SYS_SLIST_FOR_EACH_CONTAINER (&mActiveSessionsList, sessionCtx, mSessionContextNode) {
		if (sessionCtx->mSessionContextData == sessionContextData) {
			return sessionCtx;
		}
	}

	return nullptr;
}

void UltraWideBandImpl::DestroySession(SessionContext *sessionCtx)
{
	if (sessionCtx->mUwbSessionContext) {
		aliro_uwb_session_destroy(sessionCtx->mUwbSessionContext);
		sessionCtx->mUwbSessionContext = nullptr;
	}
}

} // namespace Aliro::Uwb
