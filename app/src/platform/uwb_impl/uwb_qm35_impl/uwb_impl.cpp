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

void UltraWideBandImpl::UwbCoreCallback(cherry_core_event *event, void *userData)
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
		auto sessionData = event->data;

		switch (event->type) {
		case ALIRO_UWB_SESSION_EVENT_TYPE_SESSION_STATUS:
#ifdef CONFIG_ALIRO_UWB_SESSION_LOGGING

			LOG_INF("Session status changed: %s (%d), reason: %s (%d)",
				CherryCccSessionStateToString(sessionData.status->session_state),
				sessionData.status->session_state,
				CherryCccReasonCodeToString(sessionData.status->reason_code),
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

					auto *uwbImpl = static_cast<UltraWideBandImpl *>(userData);
					auto *sessionCtx = uwbImpl->FindSession(event->session);
					VerifyOrExit(sessionCtx, LOG_WRN("Session context not found"));

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

void UltraWideBandImpl::DelayedInitWorkCallback(k_work *work)
{
	auto *delayedWork = k_work_delayable_from_work(work);
	VerifyOrReturn(delayedWork, LOG_ERR("Work pointer is null."));
	auto *sessionCtx = CONTAINER_OF(delayedWork, SessionContext, mInitiateRangingWork);

	auto &uwbImpl = UltraWideBandImpl::Instance();

	const auto *foundSession = uwbImpl.FindSession(sessionCtx->mSessionContextData);
	VerifyOrReturn(foundSession, LOG_ERR("Session context not found."));

	AliroError result = uwbImpl._InitiateRangingSession(sessionCtx->mSessionContextData);
	VerifyOrReturn(result == ALIRO_NO_ERROR, LOG_ERR("Failed to initiate ranging session: %d", result.ToInt()));

	LOG_DBG("Reader initialized ranging session for handle: %p", sessionCtx->mSessionContextData);
}

AliroError UltraWideBandImpl::_Init(const Callbacks &callbacks)
{
	cherry_err cErr{};
	uint32_t event{};

	mCallbacks = callbacks;

	VerifyOrReturnStatus(k_mutex_init(&mMutex) == 0, ALIRO_ERROR_INTERNAL, LOG_ERR("Failed to initialize mutex"));
	sys_slist_init(&mActiveSessionsList);

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

void UltraWideBandImpl::_BleTimeSync() const
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
			     LOG_ERR("Cannot handle UWB session message 0x%x", err));

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
			     LOG_ERR("Failed to initialize UWB session setup: 0x%x", err));

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
			     LOG_ERR("Failed to suspend UWB session: 0x%x", err));

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
			     LOG_ERR("Failed to resume UWB session: 0x%x", err));

	return ALIRO_NO_ERROR;
}

AliroError UltraWideBandImpl::AddSession(const SessionContext &sessionCtx)
{
	auto newCtx = Aliro::new_nothrow<SessionContext>();
	VerifyOrReturnStatus(newCtx, ALIRO_NO_MEMORY, LOG_ERR("Memory allocation failed for session context."));

	newCtx->mUwbSessionContext = sessionCtx.mUwbSessionContext;
	newCtx->mSessionContextData = sessionCtx.mSessionContextData;
	k_work_init_delayable(&newCtx->mInitiateRangingWork, DelayedInitWorkCallback);

	MutexGuard lock{ mMutex };
	constexpr k_timeout_t kUwbRangingSessionInitiationDelay =
		K_MSEC(CONFIG_ALIRO_UWB_RANGING_SESSION_INIT_DELAY_MS);
	int scheduleResult = k_work_schedule(&newCtx->mInitiateRangingWork, kUwbRangingSessionInitiationDelay);
	VerifyOrExit(scheduleResult,
		     LOG_ERR("Failed to schedule delayed work for session initiation: %d", scheduleResult));

	sys_slist_append(&mActiveSessionsList, &newCtx->mSessionContextNode);

	return ALIRO_NO_ERROR;

exit:
	delete newCtx;

	return ALIRO_ERROR_INTERNAL;
}

void UltraWideBandImpl::RemoveSession(SessionContext *sessionCtx)
{
	k_work_sync sync{};
	k_work_cancel_delayable_sync(&sessionCtx->mInitiateRangingWork, &sync);

	// TODO: Need to change to async.
	if (sessionCtx->mUwbSessionContext) {
		aliro_uwb_session_destroy(sessionCtx->mUwbSessionContext);
		sessionCtx->mUwbSessionContext = nullptr;
	}

	MutexGuard lock{ mMutex };
	if (sys_slist_find_and_remove(&mActiveSessionsList, &sessionCtx->mSessionContextNode)) {
		delete sessionCtx;
	}
}

void UltraWideBandImpl::RemoveAllSessions()
{
	SessionContext *sessionCtx{};
	SessionContext *nextSessionCtx{};

	MutexGuard lock{ mMutex };
	SYS_SLIST_FOR_EACH_CONTAINER_SAFE (&mActiveSessionsList, sessionCtx, nextSessionCtx, mSessionContextNode) {
		RemoveSession(sessionCtx);
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

} // namespace Aliro::Uwb
