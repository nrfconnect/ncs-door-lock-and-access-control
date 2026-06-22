/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "uwb_impl.h"
#include "dfu/uwb_dfu.h"
#include "session_event_hub.h"
#include "uwb_message.h"

#include <doorlock/utils/memory.h>
#include <doorlock/utils/mutex_guard.h>
#include <doorlock/utils/utils.h>

// UWB API
#include <aliro_uwb_adapter/aliro_uwb_adapter.h>
#include <aliro_uwb_adapter/aliro_uwb_session.h>

// Calibration data
#include "calibration/qm35825_calib.h"

#include <cherry/cherry_common.h>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <zephyr/sys/byteorder.h>
#include <zephyr/sys/util.h>

#include <errno.h>

LOG_MODULE_REGISTER(UwbImpl, CONFIG_DOOR_LOCK_ALIRO_UWB_LOG_LEVEL);

namespace {

using Aliro::RangingSessionState;

const char *ControleeFrameStatusStr(cherry_common_frame_status status) noexcept
{
	switch (status) {
	case CHERRY_COMMON_FRAME_STATUS_OK:
		return "OK";
	case CHERRY_COMMON_FRAME_STATUS_UNKNOWN:
		return "UNKNOWN";
	case CHERRY_COMMON_FRAME_STATUS_TX_FAILED:
		return "TX_FAILED";
	case CHERRY_COMMON_FRAME_STATUS_RX_TIMEOUT:
		return "RX_TIMEOUT";
	case CHERRY_COMMON_FRAME_STATUS_RX_PHY_DEC_FAILED:
		return "RX_PHY_DEC_FAILED";
	case CHERRY_COMMON_FRAME_STATUS_RX_PHY_TOA_FAILED:
		return "RX_PHY_TOA_FAILED";
	case CHERRY_COMMON_FRAME_STATUS_RX_PHY_STS_FAILED:
		return "RX_PHY_STS_FAILED";
	case CHERRY_COMMON_FRAME_STATUS_RX_MAC_DEC_FAILED:
		return "RX_MAC_DEC_FAILED";
	case CHERRY_COMMON_FRAME_STATUS_RX_MAC_IE_DEC_FAILED:
		return "RX_MAC_IE_DEC_FAILED";
	case CHERRY_COMMON_FRAME_STATUS_RX_MAC_IE_MISSING:
		return "RX_MAC_IE_MISSING";
	default:
		return "?";
	}
}

K_EVENT_DEFINE(sUwbEvents);

constexpr uint32_t kUwbWaitForEventsTimeoutMs{ 1000 };
constexpr uint16_t kUwbMaximumReportableDistanceCm{ 500 };

/**
 * @brief Converts aliro_uwb_err to int error code.
 *
 * @param uwbErr The UWB error code to convert.
 *
 * @return 0 on success, negative error code otherwise.
 */
constexpr int ConvertUwbError(aliro_uwb_err uwbErr)
{
	switch (uwbErr) {
	case ALIRO_UWB_ERR_NONE:
		return 0;
	case ALIRO_UWB_ERR_INVALID_PARAMETER:
	case ALIRO_UWB_ERR_MSG_MALFORMED:
		return -EINVAL;
	case ALIRO_UWB_ERR_UWBS_TIMEOUT:
		return -ETIMEDOUT;
	case ALIRO_UWB_ERR_INTERNAL:
	case ALIRO_UWB_ERR_SESSION_INIT:
		return -EIO;
	case ALIRO_UWB_ERR_SESSION_ACTIVE:
	case ALIRO_UWB_ERR_SESSION_CONFIG:
	case ALIRO_UWB_ERR_MESSAGE_STATE:
	case ALIRO_UWB_ERR_INVALID_STATE:
		return -EIO;
	default:
		return -EIO;
	}
}

#ifdef CONFIG_DOOR_LOCK_ALIRO_UWB_SESSION_LOGGING

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

#endif // CONFIG_DOOR_LOCK_ALIRO_UWB_SESSION_LOGGING

} // namespace

namespace Aliro::Uwb {

using namespace DoorLock::Utils;

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

		VerifyAndCall(uwbImpl->mCallbacks.mBleMessageTransmit, sessionCtx->mSessionContextData, message->data,
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

		SessionEventHub::DispatchSessionEvent(event, sessionCtx);

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

#ifdef CONFIG_DOOR_LOCK_ALIRO_UWB_SESSION_LOGGING

			LOG_INF("Session status changed: %s (%u) | %s (%u) -> %s (%u) [reason: %s (%u)]",
				RangingSessionStateToString(sessionCtx->mRangingSessionState),
				static_cast<uint32_t>(sessionCtx->mRangingSessionState),
				CherryCccSessionStateToString(oldState), static_cast<uint32_t>(oldState),
				CherryCccSessionStateToString(newState), static_cast<uint32_t>(newState),
				CherryCccReasonCodeToString(reason), static_cast<uint32_t>(reason));

#else // CONFIG_DOOR_LOCK_ALIRO_UWB_SESSION_LOGGING

			LOG_DBG("Session status changed: %u -> %u [reason: %u]", static_cast<uint32_t>(oldState),
				static_cast<uint32_t>(newState), static_cast<uint32_t>(reason));

#endif // CONFIG_DOOR_LOCK_ALIRO_UWB_SESSION_LOGGING

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

					if (currentMeasurement->distance_cm > kUwbMaximumReportableDistanceCm) {
						LOG_INF("Ignoring measured distance");
						break;
					}

					sys_put_be16(currentMeasurement->distance_cm,
						     uwbImpl->mCurrentDistanceCm.data());
					VerifyAndCall(uwbImpl->mCallbacks.mRangingData, sessionCtx->mSessionContextData,
						      UwbRangingData{ .mData = uwbImpl->mCurrentDistanceCm.data(),
								      .mLength = uwbImpl->mCurrentDistanceCm.size() });
				} else {
					LOG_INF("error: 0x%x (%s) slot: %d",
						static_cast<unsigned int>(currentMeasurement->frame_status),
						ControleeFrameStatusStr(static_cast<cherry_common_frame_status>(
							currentMeasurement->frame_status)),
						currentMeasurement->slot_index);
				}
				currentMeasurement = currentMeasurement->next;
			}
			break;
		}
		case ALIRO_UWB_SESSION_EVENT_TYPE_SESSION_DIAGNOSTIC_REPORT:
			break;
		default:
			break;
		}
	}

exit:
	// Free the consumed event.
	aliro_uwb_session_event_free(event);
}

int UltraWideBandImpl::HandleDeviceCapsEvent(CoreEvent *event)
{
	VerifyOrReturnValue(event && event->data.device_caps, -EINVAL,
			    LOG_ERR("Invalid event object (device capabilities)."));
	mAliroCtx = aliro_uwb_adapter_create_reader(mCtx, event->data.device_caps, &mReaderConfig);
	VerifyOrReturnValue(mAliroCtx, -EIO, LOG_ERR("Failed to create UWB adapter reader."));

	if (event->data.device_caps->ccc_capabilities) {
		const auto *src = event->data.device_caps->ccc_capabilities;
		mCccCaps.mSlotBitmask = src->slot_bitmask;
		mCccCaps.mChannelBitmask = src->channel_bitmask;
		mCccCaps.mHoppingConfigBitmask = src->hopping_config_bitmask;
		mCccCaps.mSyncCodeIndexBitmask = src->sync_code_index_bitmask;
		mCccCaps.mMinimumRanMultiplier = src->minimum_ran_multiplier;
		mCccCapsValid = true;
	}

	return 0;
}

int UltraWideBandImpl::HandleDeviceInfoEvent(CoreEvent *event)
{
	VerifyOrReturnValue(event && event->data.device_info, -EINVAL, LOG_ERR("Invalid event object (device info)."));
	const char *fwVersion = event->data.device_info->fw_version;
	const size_t fwVersionLen = std::strlen(fwVersion) + 1;

	mQm35FirmwareVersion = make_unique_array_nothrow<char>(fwVersionLen);
	VerifyOrReturnValue(mQm35FirmwareVersion, -ENOMEM,
			    LOG_ERR("Memory allocation failed for QM35 firmware version."));

	std::strcpy(mQm35FirmwareVersion.get(), fwVersion);
	LOG_INF("QM35 FW revision: %s", mQm35FirmwareVersion.get());

	return 0;
}

int UltraWideBandImpl::HandleUwbEvent(UwbEvents expectedEvent, EventHandler eventHandler)
{
	// Wait for the expected event or error to be received.
	uint32_t event = k_event_wait(&sUwbEvents, UwbEvents::All, false, K_MSEC(kUwbWaitForEventsTimeoutMs));

	int result{ -EIO };
	// Check for timeout
	VerifyOrExit(event != UwbEvents::Timeout,
		     LOG_ERR("Timeout waiting for UWB event (expected: 0x%x).", expectedEvent);
		     result = -ETIMEDOUT;);

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

int UltraWideBandImpl::_Init(const Callbacks &callbacks)
{
	auto onRadarMeasurement = [](const uint8_t *data, size_t size) {
#ifdef CONFIG_DOOR_LOCK_ALIRO_UWB_QM35_FRONT_BACK_DETECTION
		Instance().mFrontBackDetection.HandleRadarMeasurement(data, size);
#else
		LOG_INF("Radar measurement received");
#endif // CONFIG_DOOR_LOCK_ALIRO_UWB_QM35_FRONT_BACK_DETECTION
	};

	auto onSessionStopped = []() {
#ifdef CONFIG_DOOR_LOCK_ALIRO_UWB_QM35_FRONT_BACK_DETECTION
		Instance().mFrontBackDetection.CancelProcessing();
#else
		LOG_INF("Session stopped");
#endif // CONFIG_DOOR_LOCK_ALIRO_UWB_QM35_FRONT_BACK_DETECTION
	};

	LOG_INF("Initializing UWB device...");

	mInitialized = false;
	mCccCapsValid = false;

	mCallbacks = callbacks;

	VerifyOrReturnValue(k_mutex_init(&mMutex) == 0, -EIO, LOG_ERR("Failed to initialize mutex"));
	sys_slist_init(&mActiveSessionsList);

	mCtx = cherry_create("qm35", &UwbCoreCallback, this);
	VerifyOrReturnValue(mCtx, -ENODEV, LOG_ERR("Failed to create Cherry context"));

	auto err = GetDeviceInfo();

#ifdef CONFIG_DOOR_LOCK_ALIRO_UWB_QM35_DFU

	if (err != 0 || Dfu::ShouldUpdate(mQm35FirmwareVersion.get())) {
		cherry_destroy_sync(mCtx);
		mCtx = nullptr;

		const auto ret = Dfu::PerformFirmwareUpdate();
		VerifyOrReturnValue(ret == 0, -EIO, LOG_ERR("Firmware update failed"));

		mCtx = cherry_create("qm35", &UwbCoreCallback, this);
		VerifyOrReturnValue(mCtx, -ENODEV, LOG_ERR("Failed to create Cherry context"));

		err = GetDeviceInfo();
	}

#endif // CONFIG_DOOR_LOCK_ALIRO_UWB_QM35_DFU

	VerifyOrExit(err == 0);

	err = SetCalibrationData();
	VerifyOrExit(err == 0);

	err = GetDeviceCapabilities();
	VerifyOrExit(err == 0);

	PrintCccCapabilities();

#ifdef CONFIG_DOOR_LOCK_ALIRO_UWB_QM35_FRONT_BACK_DETECTION
	err = mFrontBackDetection.Init(&mActiveSessionsList, &mMutex);
	VerifyOrExit(err == 0);
#endif // CONFIG_DOOR_LOCK_ALIRO_UWB_QM35_FRONT_BACK_DETECTION

#ifdef CONFIG_DOOR_LOCK_ALIRO_UWB_QM35_RADAR
	mRadar.Init(mCtx, onRadarMeasurement, onSessionStopped);
#endif // CONFIG_DOOR_LOCK_ALIRO_UWB_QM35_RADAR

#ifdef CONFIG_DOOR_LOCK_ALIRO_UWB_QM35_DIAGNOSTICS
	mDiagnostics.Init(mAliroCtx);
#endif // CONFIG_DOOR_LOCK_ALIRO_UWB_QM35_DIAGNOSTICS

	mInitialized = true;
	LOG_INF("UWB device initialized successfully.");

	return 0;

exit:
	LOG_ERR("UWB device initialization failed");
	_Deinit();
	return -ENODEV;
}

int UltraWideBandImpl::_Deinit()
{
	mInitialized = false;
	mCccCapsValid = false;
	mQm35FirmwareVersion.reset();

	RemoveAllSessions();

	if (mAliroCtx) {
		aliro_uwb_adapter_destroy(mAliroCtx);
		mAliroCtx = nullptr;
	}

	_StopRadarSession();

	// The Cherry context is destroyed last, after all other resources.
	if (mCtx) {
		cherry_destroy_sync(mCtx);
		mCtx = nullptr;
	}

	LOG_INF("UWB device deinitialized successfully.");

	return 0;
}

void UltraWideBandImpl::_BleTimeSync()
{
	LOG_INF("Start Bluetooth LE and UWB time synchronization, procedure 0");
}

int UltraWideBandImpl::_HandleBleMessage(const uint8_t *data, size_t length, SessionContextHandle sessionContextData)
{
	VerifyOrReturnValue(data && length > 0, -EINVAL, LOG_ERR("Invalid BLE message data."));

	aliro_uwb_err err{};
	Message message(length);
	VerifyOrReturnValue(message, -ENOMEM, LOG_ERR("Memory allocation failed."));

	memcpy(message->data, data, length);
	message->len = length;

	const auto *sessionCtx = FindSession(sessionContextData);
	VerifyOrReturnValue(sessionCtx, -ENOENT,
			    LOG_ERR("Session context not found for handle: %p", sessionContextData.GetRaw()));
	err = aliro_uwb_session_message_handle(sessionCtx->mUwbSessionContext, message.get());
	VerifyOrReturnValue(err == ALIRO_UWB_ERR_NONE, ConvertUwbError(err),
			    LOG_ERR("Cannot handle UWB session message 0x%x", ToUnderlying(err)));

	return 0;
}

int UltraWideBandImpl::_ConfigureRangingSession(SessionIdentifier sessionId, const CryptoTypes::Ursk &ursk,
						ProtocolVersion protocolVersion,
						SessionContextHandle sessionContextHandle)
{
	VerifyOrReturnValue(mAliroCtx, -EBUSY, LOG_ERR("UWB is not initialized."));

	auto *sessionCtx = FindSession(sessionContextHandle);
	VerifyOrReturnValue(!sessionCtx, -EBUSY,
			    LOG_ERR("Session context already exists for handle: %p", sessionContextHandle.GetRaw()));

	aliro_uwb_session *newSessionCtx =
		aliro_uwb_session_create(mAliroCtx, sessionId, &SessionHandlerCallback, &TransmitBleMessage, this);
	VerifyOrReturnValue(newSessionCtx, -ENOMEM, LOG_ERR("Failed to create UWB session."));

	int err = ConvertUwbError(aliro_uwb_session_set_ursk(newSessionCtx, ursk.data()));
	VerifyOrExit(err == 0, LOG_ERR("Failed to set URSK in UWB session: %d", err));

	err = ConvertUwbError(aliro_uwb_session_set_protocol_version(newSessionCtx, protocolVersion));
	VerifyOrExit(err == 0, LOG_ERR("Failed to set protocol version in UWB session: %d", err));

	err = AddSession(newSessionCtx, sessionContextHandle);
	VerifyOrExit(err == 0, LOG_ERR("Failed to add session to the active sessions list: %d", err));

	LOG_INF("UWB session created with sessionContextHandle: %p", sessionContextHandle.GetRaw());
	return 0;

exit:
	aliro_uwb_session_destroy(newSessionCtx);

	return err;
}

int UltraWideBandImpl::_InitiateRangingSession(SessionContextHandle sessionContextData)
{
	const auto *sessionCtx = FindSession(sessionContextData);
	VerifyOrReturnValue(sessionCtx, -ENOENT,
			    LOG_ERR("Session context not found for handle: %p", sessionContextData.GetRaw()));

	aliro_uwb_err err = aliro_uwb_session_init_setup(sessionCtx->mUwbSessionContext);
	VerifyOrReturnValue(err == ALIRO_UWB_ERR_NONE, ConvertUwbError(err),
			    LOG_ERR("Failed to initialize UWB session setup: 0x%x", ToUnderlying(err)));

	return 0;
}

int UltraWideBandImpl::_TerminateRangingSession(SessionContextHandle sessionContextData)
{
	auto *sessionCtx = FindSession(sessionContextData);
	VerifyOrReturnValue(sessionCtx, -ENOENT,
			    LOG_ERR("Session context not found for handle: %p", sessionContextData.GetRaw()));

	RemoveSession(sessionCtx);

	LOG_DBG("Terminating UWB session with context: %p", sessionContextData.GetRaw());

	return 0;
}

int UltraWideBandImpl::_SuspendRangingSession(SessionContextHandle sessionContextData)
{
	const auto *sessionCtx = FindSession(sessionContextData);
	VerifyOrReturnValue(sessionCtx, -ENOENT,
			    LOG_ERR("Session context not found for handle: %p", sessionContextData.GetRaw()));

	aliro_uwb_err err = aliro_uwb_session_suspend(sessionCtx->mUwbSessionContext);
	VerifyOrReturnValue(err == ALIRO_UWB_ERR_NONE, ConvertUwbError(err),
			    LOG_ERR("Failed to suspend UWB session: 0x%x", ToUnderlying(err)));

	return 0;
}

int UltraWideBandImpl::_ResumeRangingSession(SessionContextHandle sessionContextData)
{
	const auto *sessionCtx = FindSession(sessionContextData);
	VerifyOrReturnValue(sessionCtx, -ENOENT,
			    LOG_ERR("Session context not found for handle: %p", sessionContextData.GetRaw()));

	aliro_uwb_err err = aliro_uwb_session_resume(sessionCtx->mUwbSessionContext);
	VerifyOrReturnValue(err == ALIRO_UWB_ERR_NONE, ConvertUwbError(err),
			    LOG_ERR("Failed to resume UWB session: 0x%x", ToUnderlying(err)));

	return 0;
}

int UltraWideBandImpl::AddSession(UwbSessionContext uwbSessionContext, SessionContextHandle sessionContextHandle)
{
	auto newCtx = new_nothrow<SessionContext>(uwbSessionContext, sessionContextHandle);
	VerifyOrReturnValue(newCtx, -ENOMEM, LOG_ERR("Memory allocation failed for session context."));

	MutexGuard lock{ mMutex };

	sys_slist_append(&mActiveSessionsList, &newCtx->mSessionContextNode);

#ifdef CONFIG_DOOR_LOCK_ALIRO_UWB_QM35_FRONT_BACK_DETECTION
	return mFrontBackDetection.AssignSessionIndex(*newCtx);
#else
	return 0;
#endif // CONFIG_DOOR_LOCK_ALIRO_UWB_QM35_FRONT_BACK_DETECTION
}

void UltraWideBandImpl::RemoveSession(SessionContext *sessionCtx)
{
	{
		MutexGuard lock{ mMutex };
		VerifyOrReturn(sys_slist_find_and_remove(&mActiveSessionsList, &sessionCtx->mSessionContextNode),
			       LOG_WRN("Session doesn't exist"));

#ifdef CONFIG_DOOR_LOCK_ALIRO_UWB_QM35_FRONT_BACK_DETECTION
		mFrontBackDetection.ReleaseSessionIndex(*sessionCtx);
#endif // CONFIG_DOOR_LOCK_ALIRO_UWB_QM35_FRONT_BACK_DETECTION
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

#ifdef CONFIG_DOOR_LOCK_ALIRO_UWB_QM35_FRONT_BACK_DETECTION
			mFrontBackDetection.ReleaseSessionIndex(*sessionCtx);
#endif // CONFIG_DOOR_LOCK_ALIRO_UWB_QM35_FRONT_BACK_DETECTION
		}

		DestroySession(sessionCtx);
		delete sessionCtx;
	}
}

int UltraWideBandImpl::_StartRadarSession()
{
#ifdef CONFIG_DOOR_LOCK_ALIRO_UWB_QM35_RADAR
	return mRadar.ScheduleStart();
#endif // CONFIG_DOOR_LOCK_ALIRO_UWB_QM35_RADAR
	return 0;
}

void UltraWideBandImpl::_StopRadarSession()
{
#ifdef CONFIG_DOOR_LOCK_ALIRO_UWB_QM35_RADAR
	mRadar.Stop();
#endif // CONFIG_DOOR_LOCK_ALIRO_UWB_QM35_RADAR
}

SessionContext *UltraWideBandImpl::FindSession(const UwbSessionContext uwbSessionCtx)
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

SessionContext *UltraWideBandImpl::FindSession(SessionContextHandle sessionContextData)
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

#ifdef CONFIG_DOOR_LOCK_ALIRO_UWB_QM35_FRONT_BACK_DETECTION
std::optional<uint8_t> UltraWideBandImpl::_GetDisambiguationSessionIdx(SessionContextHandle sessionContextData)
{
	const auto *sessionCtx = FindSession(sessionContextData);
	if (!sessionCtx) {
		return std::nullopt;
	}
	return sessionCtx->mDisambiguationSessionIdx;
}
#endif // CONFIG_DOOR_LOCK_ALIRO_UWB_QM35_FRONT_BACK_DETECTION

void UltraWideBandImpl::DestroySession(SessionContext *sessionCtx)
{
	if (sessionCtx->mUwbSessionContext) {
		aliro_uwb_session_destroy(sessionCtx->mUwbSessionContext);
		sessionCtx->mUwbSessionContext = nullptr;
	}
}

int UltraWideBandImpl::GetDeviceInfo()
{
	const auto cErr = cherry_get_device_info(mCtx);
	VerifyOrReturnValue(cErr == CHERRY_ERR_NONE, -EIO,
			    LOG_ERR("Failed to get device info: %s", cherry_err_str(cErr)));

	return HandleUwbEvent(UwbEvents::DeviceInfo,
			      [](CoreEvent *event) -> int { return Instance().HandleDeviceInfoEvent(event); });
}

int UltraWideBandImpl::GetDeviceCapabilities()
{
	const auto cErr = cherry_get_device_capabilities(mCtx);
	VerifyOrReturnValue(cErr == CHERRY_ERR_NONE, -EIO,
			    LOG_ERR("Failed to get device capabilities: %s", cherry_err_str(cErr)));

	return HandleUwbEvent(UwbEvents::DeviceCaps,
			      [](CoreEvent *event) -> int { return Instance().HandleDeviceCapsEvent(event); });
}

void UltraWideBandImpl::PrintCccCapabilities()
{
	if (mCccCapsValid) {
		LOG_INF("[UWB] CCC: slots=0x%02x ch=0x%02x hop=0x%02x sync=0x%08x ran_min=%u", mCccCaps.mSlotBitmask,
			mCccCaps.mChannelBitmask, mCccCaps.mHoppingConfigBitmask, mCccCaps.mSyncCodeIndexBitmask,
			mCccCaps.mMinimumRanMultiplier);
	} else {
		LOG_INF("[UWB] CCC: N/A");
	}
}

int UltraWideBandImpl::SetCalibrationData()
{
	// Use full calibration data for the QM35825 device.
	// This needs to be done only once during initialization, or after a power cycle.
	const auto calibData = &util_calib_qm35825;
	const auto cErr = cherry_set_calib(mCtx, calibData);
	VerifyOrReturnValue(cErr == CHERRY_ERR_NONE, -EIO,
			    LOG_ERR("Failed to set calibration data: %s", cherry_err_str(cErr)));

	return 0;
}

} // namespace Aliro::Uwb
