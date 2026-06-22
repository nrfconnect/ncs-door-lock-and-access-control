/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include "session_event_hub.h"

#include <aliro_uwb_adapter/aliro_uwb_adapter.h>

namespace Aliro::Uwb {

/**
 * @brief QM35 UWB diagnostics.
 */
class UwbDiagnostics {
public:
	/**
	 * @brief Callback function to handle the diagnostic report.
	 *
	 * @param diagnostics The diagnostic report.
	 */
	using OnDiagnosticReport = void (*)(const cherry_common_diag_report *diagnostics);

	/**
	 * @brief Initialize the diagnostics.
	 *
	 * @param ctx The Cherry context.
	 * @param onDiagnosticReport The callback function to handle the diagnostic report.
	 */
	void Init(aliro_uwb_adapter *aliroCtx, OnDiagnosticReport onDiagnosticReport = nullptr);

private:
	static void OnSessionEvent(const aliro_uwb_session_event &event, const SessionContext &sessionCtx, void *ctx);

	aliro_uwb_adapter *mAliroCtx{ nullptr };
	OnDiagnosticReport mOnDiagnosticReport{ nullptr };

	SessionEventHub::Subscriber mSubscriber{ .mOnSessionEvent = OnSessionEvent, .mCtx = this };
};

} // namespace Aliro::Uwb
