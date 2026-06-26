/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include "session_event_hub.h"

#include <doorlock/utils/mutex_guard.h>
#include <zephyr/kernel.h>

#include <array>
#include <cstddef>
#include <cstdint>

struct cherry_common_diag_report;
struct cherry_ccc_session_controlee_measurements;

namespace Aliro::Uwb {

/**
 * @brief QM35 UWB front/back detection integration.
 */
class FrontBackDetection {
public:
	/**
	 * @brief Initialize front/back detection and register session-event hooks.
	 *
	 * @param activeSessions Active UWB session list owned by UltraWideBandImpl.
	 * @param sessionsMutex Mutex that protects @p activeSessions.
	 *
	 * @return 0 on success, or a negative error code on failure.
	 */
	int Init(sys_slist_t *activeSessions, k_mutex *sessionsMutex);

	/**
	 * @brief Feed a radar CIR sample into the disambiguator.
	 *
	 * Intended to be called when a radar measurement is received.
	 *
	 * @param data The radar measurement data.
	 * @param size The size of the radar measurement data.
	 */
	void HandleRadarMeasurement(const uint8_t *data, size_t size);

	/**
	 * @brief Assign a front/back detection session index to a UWB session context.
	 *
	 * Intended to be called when a new UWB session is created.
	 *
	 * @param sessionCtx The UWB session context.
	 *
	 * @return 0 on success, negative errno otherwise.
	 */
	int AssignSessionIndex(SessionContext &sessionCtx, DoorLock::Utils::MutexGuard &);

	/**
	 * @brief Release the front/back detection session index held by @p sessionCtx.
	 *
	 * Intended to be called when a UWB session is removed.
	 *
	 * @param sessionCtx The UWB session context.
	 */
	void ReleaseSessionIndex(const SessionContext &sessionCtx, DoorLock::Utils::MutexGuard &);

	/** @brief Stop periodic front/back detection processing. */
	void CancelProcessing();

private:
	struct ProcessWork {
		k_work_delayable mDwork;
		FrontBackDetection *mOwner{ nullptr };
	};

	static void ProcessWorkHandler(k_work *work);
	static void OnSessionEvent(const aliro_uwb_session_event &event, const SessionContext &sessionCtx, void *ctx);
	static void ResetSession(const SessionContext &sessionCtx);

	void HandleSessionEvent(const aliro_uwb_session_event &event, const SessionContext &sessionCtx);
	void HandleDiagnosticReport(const SessionContext &sessionCtx, const cherry_common_diag_report *diagnostics);
	void HandleControleeReport(const SessionContext &sessionCtx,
				   const cherry_ccc_session_controlee_measurements *report);
	void ScheduleProcessing();
	void ProcessSessions(sys_slist_t *activeSessions);
	size_t CountActiveRangingSessions(sys_slist_t *activeSessions) const;

	SessionEventHub::Subscriber mSubscriber{ .mOnSessionEvent = OnSessionEvent, .mCtx = this };

	ProcessWork mProcessWork{};
	sys_slist_t *mActiveSessions{ nullptr };
	k_mutex *mSessionsMutex{ nullptr };
	std::array<SessionContext *, CONFIG_DOOR_LOCK_BLE_UWB_MAX_SESSIONS> mSessions{};
	bool mProcessingEnabled{ false };
};

} // namespace Aliro::Uwb
