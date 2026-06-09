/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "uwb_impl.h"
#include "dfu/uwb_dfu.h"
#include "uwb_message.h"

#include <doorlock/utils/memory.h>
#include <doorlock/utils/utils.h>

#include <doorlock/utils/mutex_guard.h>

// UWB API
#include <aliro_uwb_adapter/aliro_uwb_adapter.h>
#include <aliro_uwb_adapter/aliro_uwb_session.h>

// Calibration data
#include "calibration/qm35825_calib.h"

#include <cherry/cherry_common.h>

#ifdef CONFIG_DOOR_LOCK_ALIRO_UWB_QM35_DISAMBIGUATION
#include "disambiguation/disambiguation.h"
#include "disambiguation/processing.h"
#include <cherry/cherry_radar.h>
#endif // CONFIG_DOOR_LOCK_ALIRO_UWB_QM35_DISAMBIGUATION

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <zephyr/sys/byteorder.h>
#include <zephyr/sys/util.h>

#include <errno.h>

LOG_MODULE_REGISTER(UwbImpl, CONFIG_DOOR_LOCK_ALIRO_UWB_LOG_LEVEL);

#ifdef CONFIG_DOOR_LOCK_ALIRO_UWB_QM35_DISAMBIGUATION
namespace Aliro::Uwb {
void RadarStartWorkHandler(k_work *)
{
	(void)UltraWideBandImpl::Instance().StartRadarSession();
}
} // namespace Aliro::Uwb

K_WORK_DELAYABLE_DEFINE(sRadarStartDwork, Aliro::Uwb::RadarStartWorkHandler);
#endif // CONFIG_DOOR_LOCK_ALIRO_UWB_QM35_DISAMBIGUATION

namespace {

using Aliro::RangingSessionState;

struct MilliParts {
	const char *sign;
	uint32_t integer;
	uint32_t fraction;
};

constexpr MilliParts SplitMilli(int32_t milli) noexcept
{
	if (milli < 0) {
		// Convert to positive (account for INT32_MIN case).
		const uint32_t pos = static_cast<uint32_t>(-static_cast<int64_t>(milli));
		return { "-", pos / 1000U, pos % 1000U };
	}

	const uint32_t pos = static_cast<uint32_t>(milli);
	return { "", pos / 1000U, pos % 1000U };
}

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
constexpr uint16_t kUwbMaximumReportableDistance{ 50000 };

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

#ifdef CONFIG_DOOR_LOCK_ALIRO_UWB_QM35_DIAGNOSTICS_AOA

/**
 * @brief Converts a Q4.11 integer value to degrees.
 *
 * @param value The Q4.11 integer value to convert.
 *
 * @return The converted value in degrees.
 */
constexpr double IntQ411ToDegrees(int16_t value) noexcept
{
	constexpr double kPi{ 3.14159265358979323846 };
	constexpr uint16_t kResolution{ 2048 };
	constexpr double kIntQ411ToDegrees{ 180.0 / (kResolution * kPi) };

	return static_cast<double>(value) * kIntQ411ToDegrees;
}

/**
 * @brief Logs AoA in degrees using integer-only format specifiers.
 *
 * Converts the AoA from Q4.11 format to degrees and logs the result without using floating point printing.
 *
 * @param q411Aoa The AoA in Q4.11 format.
 */
void LogAoADegreesFromQ411(int16_t q411Aoa) noexcept
{
	const double deg = IntQ411ToDegrees(q411Aoa);
	// Multiply by 1000 and round to the nearest integer to get millidegrees.
	const int32_t mdeg = static_cast<int32_t>(deg >= 0.0 ? deg * 1000.0 + 0.5 : deg * 1000.0 - 0.5);

	const MilliParts parts = SplitMilli(mdeg);
	LOG_INF("AoA [degrees]: %s%u.%03u", parts.sign, static_cast<unsigned int>(parts.integer),
		static_cast<unsigned int>(parts.fraction));
}

/**
 * @brief Prints the UWB AoA measurements from the diagnostic report.
 *
 * @param aoas The array of AoA measurements.
 * @param nAoa The number of AoA measurements.
 */
void PrintDiagnosticReport(const cherry_common_aoa_measurement *aoas, unsigned int nAoa) noexcept
{
	VerifyOrReturn(aoas && nAoa > 0, LOG_ERR("Invalid AoA measurements."));

	for (unsigned int i = 0; i < nAoa; i++) {
		const auto &aoa = aoas[i];
		LOG_INF("AoA Q4.11 measurement: tdoa: %d, pdoa: %d, aoa: %d, fom: %d, type: %d", aoa.tdoa, aoa.pdoa,
			aoa.aoa, aoa.fom, aoa.type);

		LogAoADegreesFromQ411(aoa.aoa);
	}
}

/**
 * @brief Handles the diagnostic report.
 *
 * @param diagnostics The diagnostic report.
 */
void HandleDiagnosticReport(const cherry_common_diag_report *diagnostics) noexcept
{
	VerifyOrReturn(diagnostics, LOG_ERR("Invalid diagnostic report."));

	for (size_t i = 0; i < diagnostics->n_frame_report; i++) {
		const auto &frame = diagnostics->frame_report[i];
		if (frame.aoas && frame.n_aoa > 0) {
			PrintDiagnosticReport(frame.aoas, frame.n_aoa);
		}
	}
}

#endif // CONFIG_DOOR_LOCK_ALIRO_UWB_QM35_DIAGNOSTICS_AOA

#ifdef CONFIG_DOOR_LOCK_ALIRO_UWB_QM35_DISAMBIGUATION
constexpr uint32_t kRadarBurstPeriodMs{ 48 };
constexpr uint16_t kRadarSweepPeriodRstu{ 1200 };
constexpr uint8_t kRadarSweepsPerBurst{ 1 };
constexpr uint8_t kRadarSamplesPerSweep{ NB_SAMPLE_PER_SWEEP };
constexpr uint16_t kRadarNumberOfBursts{ 0 };
constexpr int16_t kRadarSweepOffset{ -3 };
constexpr uint8_t kRadarTxProfileIdx{ 0 };
constexpr cherry_common_preamble_duration kRadarPreambleDuration{ CHERRY_COMMON_PREAMBLE_DURATION_32 };
constexpr cherry_common_rframe_config kRadarRframeConfig{ CHERRY_COMMON_RFRAME_CONFIG_SP1 };
constexpr uint8_t kRadarPreambleCodeIndex{ 25 };
constexpr uint8_t kRadarAntSetId{ 3 };
constexpr uint8_t kRadarChannel{ 9 };
constexpr uint32_t kRadarSessionId{ 2 };
/* Delay before radar start after ranging becomes active (reduces controlee RX timeouts). */
constexpr int32_t kRadarStartDelayMs{ 300 };
#endif // CONFIG_DOOR_LOCK_ALIRO_UWB_QM35_DISAMBIGUATION

#ifdef CONFIG_DOOR_LOCK_ALIRO_UWB_QM35_DISAMBIGUATION
void LogDisambiguationResult(const Aliro::Uwb::Disambiguation::Result &result) noexcept
{
	/* pratio_u6 = p_ratio * 1e6 — avoids %f. Threshold 0.6 → 600000. */
	const int32_t pRatioU6 = static_cast<int32_t>(result.mPRatio * 1000000.0f);
	const int32_t pdoaMilliDeg = static_cast<int32_t>(result.mMeanPdoaDeg * 1000.0f);
	const MilliParts pdoa = SplitMilli(pdoaMilliDeg);
	const char *sideStr = result.isFront() ? "FRONT" : "BACK";

	LOG_INF("[tune] %s | dist:%3dcm | pratio_u6:%7d | cir:%4d | blk:%2d | pdoa:%s%u.%03u", sideStr,
		result.mDistanceCm, pRatioU6, result.mCir, result.mNoiseBlocks, pdoa.sign, pdoa.integer, pdoa.fraction);
}
#endif // CONFIG_DOOR_LOCK_ALIRO_UWB_QM35_DISAMBIGUATION

} // namespace

#ifdef CONFIG_DOOR_LOCK_ALIRO_UWB_QM35_DISAMBIGUATION
constexpr int32_t kDisambiguationTickPeriodMs{ 48 };

namespace Aliro::Uwb {
void DisambiguationTickWorkHandler(k_work *)
{
	auto &instance = UltraWideBandImpl::Instance();
	if (!instance.mDisambiguationTickEnabled) {
		return;
	}

	// Process disambiguation for each active ranging session.
	{
		Disambiguation::Result result{};
		UltraWideBandImpl::SessionContext *sessionCtx{};

		DoorLock::Utils::MutexGuard lock{ instance.mMutex };
		SYS_SLIST_FOR_EACH_CONTAINER (&instance.mActiveSessionsList, sessionCtx, mSessionContextNode) {
			if (sessionCtx->mRangingSessionState != RangingSessionState::Ranging &&
			    sessionCtx->mRangingSessionState != RangingSessionState::RangingResumed) {
				continue;
			}

			if (Disambiguation::Disambiguator::GetInstance().Process(
				    result, sessionCtx->mDisambiguationSessionIdx) == 0) {
				LogDisambiguationResult(result);
			}
		}
	}

	instance.ScheduleDisambiguationTick();
}
} // namespace Aliro::Uwb

K_WORK_DELAYABLE_DEFINE(sDisambiguationTickDwork, Aliro::Uwb::DisambiguationTickWorkHandler);
#endif // CONFIG_DOOR_LOCK_ALIRO_UWB_QM35_DISAMBIGUATION

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
#ifdef CONFIG_DOOR_LOCK_ALIRO_UWB_QM35_DISAMBIGUATION
						Disambiguation::Disambiguator::GetInstance().Reset(
							sessionCtx->mDisambiguationSessionIdx);
						LOG_INF("Disambiguation reset for new ranging session");
						uwbImpl->ScheduleDisambiguationTick();
#endif // CONFIG_DOOR_LOCK_ALIRO_UWB_QM35_DISAMBIGUATION
					} else if (sessionCtx->mRangingSessionState ==
						   RangingSessionState::RangingSuspended) {
						sessionCtx->mRangingSessionState = RangingSessionState::RangingResumed;
#ifdef CONFIG_DOOR_LOCK_ALIRO_UWB_QM35_DISAMBIGUATION
						uwbImpl->ScheduleDisambiguationTick();
#endif // CONFIG_DOOR_LOCK_ALIRO_UWB_QM35_DISAMBIGUATION
					}
				} else if (newState == CHERRY_CCC_SESSION_STATE_DEINIT) {
					sessionCtx->mRangingSessionState = RangingSessionState::Destroyed;
#ifdef CONFIG_DOOR_LOCK_ALIRO_UWB_QM35_DISAMBIGUATION
					uwbImpl->_StopRadarSession();
#endif // CONFIG_DOOR_LOCK_ALIRO_UWB_QM35_DISAMBIGUATION
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
#ifdef CONFIG_DOOR_LOCK_ALIRO_UWB_QM35_DISAMBIGUATION
					{
						const auto &dis =
							Disambiguation::Disambiguator::GetInstance().GetLastResult(
								sessionCtx->mDisambiguationSessionIdx);
						if (Disambiguation::Disambiguator::GetInstance().HasResult(
							    sessionCtx->mDisambiguationSessionIdx)) {
							LOG_INF("session %d | dist: %4d [cm] | %s",
								sessionCtx->mDisambiguationSessionIdx,
								currentMeasurement->distance_cm,
								dis.isFront() ? "FRONT" : "BACK");
						} else {
							LOG_INF("session %d | dist: %4d [cm] | ----",
								sessionCtx->mDisambiguationSessionIdx,
								currentMeasurement->distance_cm);
						}
					}
#else
					LOG_INF("Controlee report distance %d [cm]", currentMeasurement->distance_cm);
#endif

					if (currentMeasurement->distance_cm > kUwbMaximumReportableDistance) {
						LOG_INF("Ignoring measured distance");
#ifdef CONFIG_DOOR_LOCK_ALIRO_UWB_QM35_DISAMBIGUATION
						/* Put dummy to avoid desynchro with radar. */
						Disambiguation::Disambiguator::GetInstance().AddDistanceMeasurement(
							0, sessionCtx->mDisambiguationSessionIdx, true);
#endif // CONFIG_DOOR_LOCK_ALIRO_UWB_QM35_DISAMBIGUATION
						break;
					}

#ifdef CONFIG_DOOR_LOCK_ALIRO_UWB_QM35_DISAMBIGUATION
					Disambiguation::Disambiguator::GetInstance().AddDistanceMeasurement(
						currentMeasurement->distance_cm, sessionCtx->mDisambiguationSessionIdx,
						false);
					uwbImpl->ScheduleRadarStart();
#endif // CONFIG_DOOR_LOCK_ALIRO_UWB_QM35_DISAMBIGUATION

					sys_put_be16(currentMeasurement->distance_cm,
						     uwbImpl->mCurrentDistanceCm.data());
					VerifyAndCall(uwbImpl->mCallbacks.mRangingData, sessionCtx->mSessionContextData,
						      UwbRangingData{ .mData = uwbImpl->mCurrentDistanceCm.data(),
								      .mLength = uwbImpl->mCurrentDistanceCm.size() });
				} else {
					LOG_INF("session %d | error: 0x%x (%s) slot: %d",
						sessionCtx->mDisambiguationSessionIdx,
						static_cast<unsigned int>(currentMeasurement->frame_status),
						ControleeFrameStatusStr(static_cast<cherry_common_frame_status>(
							currentMeasurement->frame_status)),
						currentMeasurement->slot_index);
#ifdef CONFIG_DOOR_LOCK_ALIRO_UWB_QM35_DISAMBIGUATION
					/* Put dummy to avoid desynchro with radar. */
					Disambiguation::Disambiguator::GetInstance().AddDistanceMeasurement(
						0, sessionCtx->mDisambiguationSessionIdx, true);
#endif // CONFIG_DOOR_LOCK_ALIRO_UWB_QM35_DISAMBIGUATION
				}
				currentMeasurement = currentMeasurement->next;
			}
			break;
		}
#ifdef CONFIG_DOOR_LOCK_ALIRO_UWB_QM35_DISAMBIGUATION
		case ALIRO_UWB_SESSION_EVENT_TYPE_SESSION_DIAGNOSTIC_REPORT: {
#ifdef CONFIG_DOOR_LOCK_ALIRO_UWB_QM35_DIAGNOSTICS_AOA
			HandleDiagnosticReport(sessionData.diagnostics);
#endif // CONFIG_DOOR_LOCK_ALIRO_UWB_QM35_DIAGNOSTICS_AOA

			// Aliro CCC with 1 ranging round produces 2 frame reports; PDOA is in frame[1]
			// (controlee reply). The Qorvo demo uses 5 rounds and reads frame[4].
			if (!sessionData.diagnostics || sessionData.diagnostics->n_frame_report < 2) {
				// Put dummy to avoid desynchro with radar.
				Disambiguation::Disambiguator::GetInstance().AddPdoaMeasurement(
					0, 0, sessionCtx->mDisambiguationSessionIdx, true, true);
				break;
			}

			if (sessionData.diagnostics->n_frame_report >= 2) {
				const auto &frame = sessionData.diagnostics->frame_report[1];

				int16_t pdoa = 0;
				int16_t rssi = 0;
				bool pdoaError = true;
				bool rssiError = true;

				if (frame.aoas && frame.n_aoa > 0) {
					pdoa = frame.aoas[0].pdoa;
					pdoaError = false;
				}

				if (frame.seg_metrics && frame.n_seg_metrics > 0) {
					rssi = frame.seg_metrics[0].rsl_q8;
					rssiError = false;
				}

				Disambiguation::Disambiguator::GetInstance().AddPdoaMeasurement(
					pdoa, rssi, sessionCtx->mDisambiguationSessionIdx, pdoaError, rssiError);
			} else {
				Disambiguation::Disambiguator::GetInstance().AddPdoaMeasurement(
					0, 0, sessionCtx->mDisambiguationSessionIdx, true, true);
			}
			break;
		}
#endif // CONFIG_DOOR_LOCK_ALIRO_UWB_QM35_DISAMBIGUATION
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

#ifdef CONFIG_DOOR_LOCK_ALIRO_UWB_QM35_DISAMBIGUATION
	aliro_uwb_adapter_set_diagnostics(mAliroCtx, { .aoa = true, .rssi = true });
#ifdef CONFIG_DOOR_LOCK_ALIRO_UWB_QM35_DIAGNOSTICS_AOA
	LOG_INF("AoA diagnostics enabled");
#endif // CONFIG_DOOR_LOCK_ALIRO_UWB_QM35_DIAGNOSTICS_AOA

	Disambiguation::Disambiguator::GetInstance().Init();
	mRadarSession = nullptr;
	mRadarRunning = false;
	LOG_INF("Disambiguation initialized (Qorvo disambiguation_demo defaults)");
#endif // CONFIG_DOOR_LOCK_ALIRO_UWB_QM35_DISAMBIGUATION

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

#ifdef CONFIG_DOOR_LOCK_ALIRO_UWB_QM35_DISAMBIGUATION
	_StopRadarSession();
#endif // CONFIG_DOOR_LOCK_ALIRO_UWB_QM35_DISAMBIGUATION

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

	return 0;
}

#ifdef CONFIG_DOOR_LOCK_ALIRO_UWB_QM35_DISAMBIGUATION
void UltraWideBandImpl::ScheduleRadarStart()
{
	if (mRadarRunning) {
		return;
	}

	const int ret = k_work_schedule(&sRadarStartDwork, K_MSEC(kRadarStartDelayMs));
	if (ret > 0) {
		LOG_DBG("Radar start already pending, not rescheduled");
	} else {
		LOG_DBG("Radar start scheduled in %d ms", kRadarStartDelayMs);
	}
}

void UltraWideBandImpl::CancelRadarStart()
{
	(void)k_work_cancel_delayable(&sRadarStartDwork);
}

void UltraWideBandImpl::ScheduleDisambiguationTick()
{
	mDisambiguationTickEnabled = true;
	(void)k_work_schedule(&sDisambiguationTickDwork, K_MSEC(kDisambiguationTickPeriodMs));
}

void UltraWideBandImpl::CancelDisambiguationTick()
{
	mDisambiguationTickEnabled = false;
	(void)k_work_cancel_delayable(&sDisambiguationTickDwork);
}

void UltraWideBandImpl::RadarCallback(cherry_radar_event *event, void *userData)
{
	if (!event || !userData) {
		return;
	}
	auto *uwbImpl = static_cast<UltraWideBandImpl *>(userData);

	switch (event->type) {
	case CHERRY_RADAR_EVENT_TYPE_SESSION_REPORT: {
		auto *r = event->data.report;
		if (r && r->n_sweeps > 0 && r->sweeps && r->sweeps[0].n_data_fragments > 0 &&
		    r->sweeps[0].data_fragments && r->sweeps[0].data_fragments[0].data) {
			Disambiguation::Disambiguator::GetInstance().AddCirMeasurement(
				r->sweeps[0].data_fragments[0].data, r->sweeps[0].data_fragments[0].size);
		}
		break;
	}
	case CHERRY_RADAR_EVENT_TYPE_SESSION_STATUS:
		break;
	case CHERRY_RADAR_EVENT_TYPE_SESSION_ERROR:
		LOG_WRN("Radar session error: 0x%x", static_cast<uint32_t>(event->data.error->status_err));
		uwbImpl->_StopRadarSession();
		break;
	default:
		break;
	}

	cherry_radar_event_free(event);
}

int UltraWideBandImpl::StartRadarSession()
{
	if (mRadarRunning) {
		return 0;
	}
	VerifyOrReturnValue(mCtx, -EINVAL, LOG_WRN("Cherry context not ready for radar"));

	if (!mRadarSession) {
		mRadarSession =
			cherry_radar_session_create(mCtx, &RadarCallback, this, kRadarSessionId, kRadarBurstPeriodMs,
						    kRadarSweepPeriodRstu, kRadarSweepsPerBurst, kRadarSamplesPerSweep,
						    kRadarAntSetId);
		VerifyOrReturnValue(mRadarSession, -EIO, LOG_ERR("cherry_radar_session_create failed"));

		VerifyOrReturnValue(cherry_radar_session_set_rframe_config(mRadarSession, kRadarRframeConfig) ==
					    CHERRY_ERR_NONE,
				    -EIO, LOG_ERR("radar set_rframe_config failed"));
		VerifyOrReturnValue(cherry_radar_session_set_preamble_code_index(
					    mRadarSession, kRadarPreambleCodeIndex) == CHERRY_ERR_NONE,
				    -EIO, LOG_ERR("radar set_preamble_code_index failed"));
		VerifyOrReturnValue(cherry_radar_session_set_preamble_duration(mRadarSession, kRadarPreambleDuration) ==
					    CHERRY_ERR_NONE,
				    -EIO, LOG_ERR("radar set_preamble_duration failed"));
		VerifyOrReturnValue(cherry_radar_session_set_number_of_bursts(mRadarSession, kRadarNumberOfBursts) ==
					    CHERRY_ERR_NONE,
				    -EIO, LOG_ERR("radar set_number_of_bursts failed"));
		VerifyOrReturnValue(cherry_radar_session_set_sweep_offset(mRadarSession, kRadarSweepOffset) ==
					    CHERRY_ERR_NONE,
				    -EIO, LOG_ERR("radar set_sweep_offset failed"));
		VerifyOrReturnValue(cherry_radar_session_set_tx_profile_idx(mRadarSession, kRadarTxProfileIdx) ==
					    CHERRY_ERR_NONE,
				    -EIO, LOG_ERR("radar set_tx_profile_idx failed"));
		VerifyOrReturnValue(cherry_radar_session_set_channel(mRadarSession, kRadarChannel) == CHERRY_ERR_NONE,
				    -EIO, LOG_ERR("radar set_channel failed"));
	}

	VerifyOrReturnValue(cherry_radar_session_start(mRadarSession) == CHERRY_ERR_NONE, -EIO,
			    LOG_ERR("cherry_radar_session_start failed"));

	mRadarRunning = true;
	LOG_INF("Radar session started");
	return 0;
}

void UltraWideBandImpl::_StopRadarSession()
{
	CancelRadarStart();
	CancelDisambiguationTick();

	if (!mRadarRunning && !mRadarSession) {
		return;
	}

	if (mRadarSession) {
		cherry_radar_session_destroy(mRadarSession);
		mRadarSession = nullptr;
	}
	mRadarRunning = false;
	LOG_INF("Radar session stopped");
}
#endif // CONFIG_DOOR_LOCK_ALIRO_UWB_QM35_DISAMBIGUATION

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

#ifdef CONFIG_DOOR_LOCK_ALIRO_UWB_QM35_DISAMBIGUATION
	// Allocate a stable disambiguation session index for this session.
	{
		bool allocated = false;
		MutexGuard lock{ mMutex };
		for (uint8_t i = 0; i < mDisambiguationSessions.size(); i++) {
			if (!mDisambiguationSessions[i]) {
				mDisambiguationSessions[i] = newCtx;
				newCtx->mDisambiguationSessionIdx = i;
				allocated = true;
				break;
			}
		}
		if (!allocated) {
			delete newCtx;
			return -ENOSPC;
		}
	}
#endif // CONFIG_DOOR_LOCK_ALIRO_UWB_QM35_DISAMBIGUATION

	MutexGuard lock{ mMutex };

	sys_slist_append(&mActiveSessionsList, &newCtx->mSessionContextNode);

	return 0;
}

void UltraWideBandImpl::RemoveSession(SessionContext *sessionCtx)
{
	{
		MutexGuard lock{ mMutex };
		VerifyOrReturn(sys_slist_find_and_remove(&mActiveSessionsList, &sessionCtx->mSessionContextNode),
			       LOG_WRN("Session doesn't exist"));

#ifdef CONFIG_DOOR_LOCK_ALIRO_UWB_QM35_DISAMBIGUATION
		if (sessionCtx->mDisambiguationSessionIdx < mDisambiguationSessions.size()) {
			Disambiguation::Disambiguator::GetInstance().Reset(sessionCtx->mDisambiguationSessionIdx);
			mDisambiguationSessions[sessionCtx->mDisambiguationSessionIdx] = nullptr;
		}
#endif // CONFIG_DOOR_LOCK_ALIRO_UWB_QM35_DISAMBIGUATION
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

#ifdef CONFIG_DOOR_LOCK_ALIRO_UWB_QM35_DISAMBIGUATION
			if (sessionCtx->mDisambiguationSessionIdx < mDisambiguationSessions.size()) {
				mDisambiguationSessions[sessionCtx->mDisambiguationSessionIdx] = nullptr;
			}
#endif // CONFIG_DOOR_LOCK_ALIRO_UWB_QM35_DISAMBIGUATION
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

#ifdef CONFIG_DOOR_LOCK_ALIRO_UWB_QM35_DISAMBIGUATION
std::optional<uint8_t> UltraWideBandImpl::_GetDisambiguationSessionIdx(SessionContextHandle sessionContextData)
{
	const auto *sessionCtx = FindSession(sessionContextData);
	if (!sessionCtx) {
		return std::nullopt;
	}
	return sessionCtx->mDisambiguationSessionIdx;
}
#endif // CONFIG_DOOR_LOCK_ALIRO_UWB_QM35_DISAMBIGUATION

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
