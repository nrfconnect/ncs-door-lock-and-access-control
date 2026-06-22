/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "radar.h"

#include <doorlock/utils/utils.h>

#include <aliro/types.h>

#include <aliro_uwb_adapter/aliro_uwb_adapter.h>
#include <aliro_uwb_adapter/aliro_uwb_session.h>

#include <cherry/cherry.h>
#include <cherry/cherry_radar.h>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <tuple>

LOG_MODULE_REGISTER(UwbRadar, CONFIG_DOOR_LOCK_ALIRO_UWB_QM35_RADAR_LOG_LEVEL);

namespace {

constexpr uint32_t kRadarBurstPeriodMs{ 48 };
constexpr uint16_t kRadarSweepPeriodRstu{ 1200 };
constexpr uint8_t kRadarSweepsPerBurst{ 1 };
constexpr uint8_t kRadarSamplesPerSweep{ 32 };
constexpr uint16_t kRadarNumberOfBursts{ 0 };
constexpr int16_t kRadarSweepOffset{ -3 };
constexpr uint8_t kRadarTxProfileIdx{ 0 };
constexpr cherry_common_preamble_duration kRadarPreambleDuration{ CHERRY_COMMON_PREAMBLE_DURATION_32 };
constexpr cherry_common_rframe_config kRadarRframeConfig{ CHERRY_COMMON_RFRAME_CONFIG_SP1 };
constexpr uint8_t kRadarPreambleCodeIndex{ 25 };
constexpr uint8_t kRadarAntSetId{ CONFIG_DOOR_LOCK_ALIRO_UWB_QM35_RADAR_ANT_SET_ID };
constexpr uint8_t kRadarChannel{ CONFIG_DOOR_LOCK_ALIRO_UWB_QM35_RADAR_CHANNEL };
constexpr uint32_t kRadarSessionId{ 2 };

constexpr uint16_t kUwbMaximumReportableDistanceCm{ 500 };
constexpr int32_t kRadarStartDelayMs{ 300 };
constexpr uint32_t kRadarActivationDistanceCm{ CONFIG_DOOR_LOCK_ALIRO_UWB_QM35_RADAR_ACTIVATION_DISTANCE_CM };

} // namespace

namespace Aliro::Uwb {

void UwbRadar::Init(cherry *ctx, OnRadarMeasurement onRadarMeasurement, OnSessionStopped onSessionStopped)
{
	mCtx = ctx;
	mSession = nullptr;
	mRunning = false;
	mOnRadarMeasurement = onRadarMeasurement;
	mOnSessionStopped = onSessionStopped;

	k_work_init_delayable(&mStartWork.mDwork, StartWorkHandler);
	mStartWork.mOwner = this;

	SessionEventHub::Register(mSubscriber);
}

int UwbRadar::ScheduleStart()
{
	VerifyOrReturnValue(!mRunning, -EALREADY, LOG_ERR("Radar session already running"));
	const int ret = k_work_schedule(&mStartWork.mDwork, K_MSEC(kRadarStartDelayMs));
	VerifyOrReturnValue(ret >= 0, ret);
	return 0;
}

void UwbRadar::Stop()
{
	CancelStart();

	VerifyAndCall(mOnSessionStopped);
	VerifyOrReturn(mRunning && mSession);

	cherry_radar_session_destroy(mSession);

	mSession = nullptr;
	mRunning = false;

	LOG_INF("Radar session stopped");
}

void UwbRadar::CancelStart()
{
	k_work_sync sync;
	(void)k_work_cancel_delayable_sync(&mStartWork.mDwork, &sync);
}

void UwbRadar::StartWorkHandler(k_work *work)
{
	auto *dwork = k_work_delayable_from_work(work);
	auto *startWork = CONTAINER_OF(dwork, StartWork, mDwork);

	VerifyOrReturn(startWork->mOwner);
	startWork->mOwner->StartSession();
}

int UwbRadar::StartSession()
{
	VerifyOrReturnValue(!mRunning, 0);
	VerifyOrReturnValue(mCtx, -EINVAL, LOG_WRN("Cherry context not ready for radar"));
	VerifyOrReturnValue(!mSession, -EIO, LOG_ERR("Radar session already created"));

	mSession = cherry_radar_session_create(mCtx, &RadarCallback, this, kRadarSessionId, kRadarBurstPeriodMs,
					       kRadarSweepPeriodRstu, kRadarSweepsPerBurst, kRadarSamplesPerSweep,
					       kRadarAntSetId);
	VerifyOrReturnValue(mSession, -EIO, LOG_ERR("cherry_radar_session_create failed"));

	VerifyOrExit(cherry_radar_session_set_rframe_config(mSession, kRadarRframeConfig) == CHERRY_ERR_NONE,
		     LOG_ERR("radar set_rframe_config failed"));
	VerifyOrExit(cherry_radar_session_set_preamble_code_index(mSession, kRadarPreambleCodeIndex) == CHERRY_ERR_NONE,
		     LOG_ERR("radar set_preamble_code_index failed"));
	VerifyOrExit(cherry_radar_session_set_preamble_duration(mSession, kRadarPreambleDuration) == CHERRY_ERR_NONE,
		     LOG_ERR("radar set_preamble_duration failed"));
	VerifyOrExit(cherry_radar_session_set_number_of_bursts(mSession, kRadarNumberOfBursts) == CHERRY_ERR_NONE,
		     LOG_ERR("radar set_number_of_bursts failed"));
	VerifyOrExit(cherry_radar_session_set_sweep_offset(mSession, kRadarSweepOffset) == CHERRY_ERR_NONE,
		     LOG_ERR("radar set_sweep_offset failed"));
	VerifyOrExit(cherry_radar_session_set_tx_profile_idx(mSession, kRadarTxProfileIdx) == CHERRY_ERR_NONE,
		     LOG_ERR("radar set_tx_profile_idx failed"));
	VerifyOrExit(cherry_radar_session_set_channel(mSession, kRadarChannel) == CHERRY_ERR_NONE,
		     LOG_ERR("radar set_channel failed"));
	VerifyOrExit(cherry_radar_session_start(mSession) == CHERRY_ERR_NONE,
		     LOG_ERR("cherry_radar_session_start failed"));

	mRunning = true;
	LOG_INF("Radar session started");

	return 0;

exit:
	cherry_radar_session_destroy(mSession);
	mSession = nullptr;
	return -EIO;
}

void UwbRadar::OnSessionEvent(const aliro_uwb_session_event &event, const SessionContext &sessionCtx, void *ctx)
{
	VerifyOrReturn(ctx);

	auto *radar = static_cast<UwbRadar *>(ctx);
	radar->HandleSessionEvent(event, sessionCtx);
}

void UwbRadar::HandleSessionEvent(const aliro_uwb_session_event &event, const SessionContext &sessionCtx)
{
	switch (event.type) {
	case ALIRO_UWB_SESSION_EVENT_TYPE_SESSION_STATUS: {
		const auto *status = event.data.status;
		const auto oldState = sessionCtx.mSessionState;
		const auto newState = status->session_state;

		if (oldState == CHERRY_CCC_SESSION_STATE_IDLE && newState == CHERRY_CCC_SESSION_STATE_DEINIT) {
			Stop();
		} else if (oldState == CHERRY_CCC_SESSION_STATE_ACTIVE && newState == CHERRY_CCC_SESSION_STATE_IDLE) {
			Stop();
		}
		break;
	}
	case ALIRO_UWB_SESSION_EVENT_TYPE_SESSION_ERROR:
		Stop();
		break;
	case ALIRO_UWB_SESSION_EVENT_TYPE_SESSION_CONTROLEE_REPORT: {
		const auto *report = event.data.controlee_report;
		if (!report) {
			break;
		}
		for (const auto *m = report->measurements; m; m = m->next) {
			if (m->frame_status) {
				continue;
			}
			if (m->distance_cm > kUwbMaximumReportableDistanceCm) {
				break;
			}
			if (m->distance_cm <= kRadarActivationDistanceCm) {
				std::ignore = ScheduleStart();
			}
		}
		break;
	}
	default:
		break;
	}
}

void UwbRadar::RadarCallback(cherry_radar_event *event, void *userData)
{
	VerifyOrReturn(event);

	UwbRadar *radar{};
	VerifyOrExit(userData);
	radar = static_cast<UwbRadar *>(userData);

	switch (event->type) {
	case CHERRY_RADAR_EVENT_TYPE_SESSION_REPORT: {
		const auto *report = event->data.report;
		VerifyOrExit(report);
		VerifyOrExit(report->n_sweeps > 0);
		VerifyOrExit(report->sweeps);
		const auto sweep = report->sweeps[0];
		VerifyOrExit(sweep.n_data_fragments > 0);
		VerifyOrExit(sweep.data_fragments);
		const auto dataFragment = sweep.data_fragments[0];
		VerifyAndCall(radar->mOnRadarMeasurement, dataFragment.data, dataFragment.size);
		break;
	}
	case CHERRY_RADAR_EVENT_TYPE_SESSION_STATUS:
		break;
	case CHERRY_RADAR_EVENT_TYPE_SESSION_ERROR:
		LOG_WRN("Radar session error: 0x%x", static_cast<uint32_t>(event->data.error->status_err));
		radar->Stop();
		break;
	default:
		break;
	}
exit:
	cherry_radar_event_free(event);
}

} // namespace Aliro::Uwb
