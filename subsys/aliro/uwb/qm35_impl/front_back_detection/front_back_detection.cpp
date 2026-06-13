/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "front_back_detection.h"

#include <disambiguator.h>
#include <uwb_utils.h>

#include <aliro_uwb_adapter/aliro_uwb_adapter.h>
#include <aliro_uwb_adapter/aliro_uwb_session.h>

#include <cherry/cherry_ccc.h>
#include <cherry/cherry_common.h>

#include <doorlock/utils/mutex_guard.h>
#include <doorlock/utils/utils.h>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/slist.h>

#include <errno.h>

LOG_MODULE_REGISTER(FrontBackDetection, CONFIG_DOOR_LOCK_ALIRO_UWB_QM35_FRONT_BACK_DETECTION_LOG_LEVEL);

namespace {

constexpr uint16_t kUwbMaximumReportableDistanceCm{ 500 };

constexpr disambiguation_parameters kFrontBackDetectionParams = {
	.max_cir_threshold = CONFIG_DOOR_LOCK_ALIRO_UWB_DISAMBIGUATION_MAX_CIR,
	.p_ratio_threshold = CONFIG_DOOR_LOCK_ALIRO_UWB_DISAMBIGUATION_P_RATIO / 10000.0f,
	.nb_blocks_threshold = CONFIG_DOOR_LOCK_ALIRO_UWB_DISAMBIGUATION_NB_BLOCKS,
	.radar_distance = 0,
	.noise_threshold = CONFIG_DOOR_LOCK_ALIRO_UWB_DISAMBIGUATION_NOISE_THRESHOLD,
	.shadow_noise_threshold = CONFIG_DOOR_LOCK_ALIRO_UWB_DISAMBIGUATION_SHADOW_THRESHOLD,
	.blind_distance = CONFIG_DOOR_LOCK_ALIRO_UWB_DISAMBIGUATION_BLIND_DISTANCE_CM,
	.secure_bubble_radius = CONFIG_DOOR_LOCK_ALIRO_UWB_DISAMBIGUATION_SECURE_BUBBLE_CM,
};

void LogFrontBackDetectionResult(const Aliro::Uwb::Disambiguation::Result &result) noexcept
{
	using namespace Aliro::Uwb::Utils;

	/* pratio_u6 = p_ratio * 1e6 — avoids %f. Threshold 0.6 → 600000. */
	const int32_t pRatioU6 = static_cast<int32_t>(result.mPRatio * 1000000.0f);
	const int32_t pdoaMilliDeg = static_cast<int32_t>(result.mMeanPdoaDeg * 1000.0f);
	const auto pdoa = SplitMilli(pdoaMilliDeg);

	const char *sideStr = result.IsFront() ? "FRONT" : "BACK";

	LOG_INF("[side] %s | dist:%3dcm | pratio_u6:%7d | cir:%4d | blk:%2d | pdoa:%s%u.%03u", sideStr,
		result.mDistanceCm, pRatioU6, result.mCir, result.mNoiseBlocks, pdoa.mSign, pdoa.mInteger,
		pdoa.mFraction);
}

} // namespace

namespace Aliro::Uwb {

int FrontBackDetection::Init(sys_slist_t *activeSessions, k_mutex *sessionsMutex)
{
	VerifyOrReturnValue(activeSessions && sessionsMutex, -EINVAL);

	mActiveSessions = activeSessions;
	mSessionsMutex = sessionsMutex;
	mProcessingEnabled = false;

	int err = Disambiguation::Disambiguator::Instance().Init(kFrontBackDetectionParams);
	VerifyOrReturnValue(err == 0, err, LOG_ERR("Failed to initialize disambiguation: %d", err));

	LOG_INF("Front/back detection initialized");

	k_work_init_delayable(&mProcessWork.mDwork, ProcessWorkHandler);
	mProcessWork.mOwner = this;

	SessionEventHub::Register(mSubscriber);

	return 0;
}

void FrontBackDetection::HandleRadarMeasurement(const uint8_t *data, size_t size)
{
	VerifyOrReturn(data && size > 0);
	Disambiguation::Disambiguator::Instance().AddCirMeasurement(const_cast<uint8_t *>(data),
								    static_cast<uint16_t>(size));
}

int FrontBackDetection::AssignSessionIndex(SessionContext &sessionCtx)
{
	for (uint8_t i = 0; i < mSessions.size(); i++) {
		if (!mSessions[i]) {
			mSessions[i] = &sessionCtx;
			sessionCtx.mDisambiguationSessionIdx = i;
			return 0;
		}
	}

	return -ENOSPC;
}

void FrontBackDetection::ReleaseSessionIndex(const SessionContext &sessionCtx)
{
	if (sessionCtx.mDisambiguationSessionIdx < mSessions.size()) {
		Disambiguation::Disambiguator::Instance().ResetSession(sessionCtx.mDisambiguationSessionIdx);
		mSessions[sessionCtx.mDisambiguationSessionIdx] = nullptr;
	}
}

void FrontBackDetection::CancelProcessing()
{
	mProcessingEnabled = false;
	k_work_sync sync;
	(void)k_work_cancel_delayable_sync(&mProcessWork.mDwork, &sync);
}

void FrontBackDetection::ProcessWorkHandler(k_work *work)
{
	auto *dwork = k_work_delayable_from_work(work);
	auto *processWork = CONTAINER_OF(dwork, ProcessWork, mDwork);

	VerifyOrReturn(processWork->mOwner && processWork->mOwner->mActiveSessions);
	processWork->mOwner->ProcessSessions(processWork->mOwner->mActiveSessions);
}

void FrontBackDetection::ProcessSessions(sys_slist_t *activeSessions)
{
	if (!mProcessingEnabled || !activeSessions || !mSessionsMutex) {
		return;
	}

	Disambiguation::Result result{};

	SessionContext *sessionCtx{};
	{
		DoorLock::Utils::MutexGuard lock{ *mSessionsMutex };
		SYS_SLIST_FOR_EACH_CONTAINER (activeSessions, sessionCtx, mSessionContextNode) {
			if (sessionCtx->mRangingSessionState != RangingSessionState::Ranging &&
			    sessionCtx->mRangingSessionState != RangingSessionState::RangingResumed) {
				continue;
			}

			if (Disambiguation::Disambiguator::Instance().Process(
				    result, sessionCtx->mDisambiguationSessionIdx) == 0) {
				LogFrontBackDetectionResult(result);
			}
		}
	}

	ScheduleProcessing();
}

void FrontBackDetection::ScheduleProcessing()
{
	mProcessingEnabled = true;
	(void)k_work_schedule(&mProcessWork.mDwork,
			      K_MSEC(CONFIG_DOOR_LOCK_ALIRO_UWB_QM35_FRONT_BACK_DETECTION_PROCESSING_INTERVAL_MS));
}

void FrontBackDetection::OnSessionEvent(const aliro_uwb_session_event &event, const SessionContext &sessionCtx,
					void *ctx)
{
	VerifyOrReturn(ctx);

	static_cast<FrontBackDetection *>(ctx)->HandleSessionEvent(event, sessionCtx);
}

void FrontBackDetection::HandleSessionEvent(const aliro_uwb_session_event &event, const SessionContext &sessionCtx)
{
	switch (event.type) {
	case ALIRO_UWB_SESSION_EVENT_TYPE_SESSION_STATUS: {
		const auto *status = event.data.status;
		const auto oldState = sessionCtx.mSessionState;
		const auto newState = status->session_state;

		if (oldState == CHERRY_CCC_SESSION_STATE_IDLE && newState == CHERRY_CCC_SESSION_STATE_ACTIVE) {
			if (sessionCtx.mRangingSessionState == RangingSessionState::Idle) {
				Disambiguation::Disambiguator::Instance().ResetSession(
					sessionCtx.mDisambiguationSessionIdx);
				LOG_INF("Front/back detection reset for new ranging session");
				ScheduleProcessing();
			} else if (sessionCtx.mRangingSessionState == RangingSessionState::RangingSuspended) {
				ScheduleProcessing();
			}
		} else if (oldState == CHERRY_CCC_SESSION_STATE_ACTIVE && newState == CHERRY_CCC_SESSION_STATE_IDLE) {
			CancelProcessing();
		}
		break;
	}
	case ALIRO_UWB_SESSION_EVENT_TYPE_SESSION_CONTROLEE_REPORT: {
		const auto *report = event.data.controlee_report;
		if (report) {
			HandleControleeReport(sessionCtx, report->measurements);
		}
		break;
	}
	case ALIRO_UWB_SESSION_EVENT_TYPE_SESSION_DIAGNOSTIC_REPORT:
		HandleDiagnosticReport(sessionCtx, event.data.diagnostics);
		break;
	default:
		break;
	}
}

void FrontBackDetection::HandleControleeReport(const SessionContext &sessionCtx,
					       const cherry_ccc_session_controlee_measurements *measurements)
{
	for (const auto *m = measurements; m; m = m->next) {
		if (m->frame_status) {
			Disambiguation::Disambiguator::Instance().AddDistanceMeasurement(
				0, sessionCtx.mDisambiguationSessionIdx, true);
			continue;
		}

		if (m->distance_cm > kUwbMaximumReportableDistanceCm) {
			LOG_INF("Ignoring measurements above maximum reportable distance");
			Disambiguation::Disambiguator::Instance().AddDistanceMeasurement(
				0, sessionCtx.mDisambiguationSessionIdx, true);
			break;
		}

		Disambiguation::Disambiguator::Instance().AddDistanceMeasurement(
			m->distance_cm, sessionCtx.mDisambiguationSessionIdx, false);

		if (!mProcessingEnabled) {
			ScheduleProcessing();
		}
	}
}

void FrontBackDetection::HandleDiagnosticReport(const SessionContext &sessionCtx,
						const cherry_common_diag_report *diagnostics)
{
	// Aliro CCC with 1 ranging round produces 2 frame reports; PDOA is in frame[1]
	// (controlee reply). The Qorvo demo uses 5 rounds and reads frame[4].
	if (!diagnostics || diagnostics->n_frame_report < 2) {
		Disambiguation::Disambiguator::Instance().AddPdoaMeasurement(0, 0, sessionCtx.mDisambiguationSessionIdx,
									     true, true);
		return;
	}

	const auto &frame = diagnostics->frame_report[1];

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

	Disambiguation::Disambiguator::Instance().AddPdoaMeasurement(pdoa, rssi, sessionCtx.mDisambiguationSessionIdx,
								     pdoaError, rssiError);
}

} // namespace Aliro::Uwb
