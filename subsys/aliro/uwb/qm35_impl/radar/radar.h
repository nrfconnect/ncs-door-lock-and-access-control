/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include "session_event_hub.h"

#include <zephyr/kernel.h>

struct aliro_uwb_session;
struct cherry_radar_event;
struct cherry_radar_session;
struct cherry;

namespace Aliro::Uwb {

/**
 * @brief QM35 Cherry radar session and session-event hooks.
 */
class UwbRadar {
public:
	/**
	 * @brief Callback function to handle the radar measurement.
	 *
	 * @param data The radar measurement data.
	 * @param size The size of the radar measurement data.
	 */
	using OnRadarMeasurement = void (*)(const uint8_t *data, size_t size);

	/** @brief Callback invoked when the radar session is stopped. */
	using OnSessionStopped = void (*)();

	/**
	 * @brief Initialize the radar session.
	 *
	 * @param ctx The Cherry context.
	 * @param onRadarMeasurement The callback function to handle the radar measurement.
	 * @param onSessionStopped Optional callback when the radar session stops.
	 */
	void Init(cherry *ctx, OnRadarMeasurement onRadarMeasurement = nullptr,
		  OnSessionStopped onSessionStopped = nullptr);

	/**
	 * @brief Stop an active radar session and cancel any pending start.
	 */
	void Stop();

	/** @brief Schedule radar session start after the empirically determined delay.
	 *
	 * @return 0 on success, negative error code otherwise.
	 */
	int ScheduleStart();

private:
	struct StartWork {
		k_work_delayable mDwork;
		UwbRadar *mOwner{ nullptr };
	};

	static void StartWorkHandler(k_work *work);
	static void OnSessionEvent(const aliro_uwb_session_event &event, const SessionContext &sessionCtx, void *ctx);
	static void RadarCallback(cherry_radar_event *event, void *userData);

	void CancelStart();
	int StartSession();
	void HandleSessionEvent(const aliro_uwb_session_event &event, const SessionContext &sessionCtx);

	cherry *mCtx{ nullptr };
	cherry_radar_session *mSession{ nullptr };
	bool mRunning{ false };
	StartWork mStartWork{};
	OnRadarMeasurement mOnRadarMeasurement{ nullptr };
	OnSessionStopped mOnSessionStopped{ nullptr };

	SessionEventHub::Subscriber mSubscriber{ .mOnSessionEvent = OnSessionEvent, .mCtx = this };
};

} // namespace Aliro::Uwb
