/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "diagnostics.h"

#include <uwb_utils.h>

#include <aliro/utils.h>

#include <aliro_uwb_adapter/aliro_uwb_session.h>

#include <cherry/cherry_common.h>

#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(UwbDiagnostics, CONFIG_DOOR_LOCK_ALIRO_UWB_QM35_DIAGNOSTICS_LOG_LEVEL);

#if CONFIG_DOOR_LOCK_ALIRO_UWB_QM35_DIAGNOSTICS_DEFAULT_HANDLER
namespace {

constexpr double IntQ411ToDegrees(int16_t value) noexcept
{
	constexpr double kPi{ 3.14159265358979323846 };
	constexpr uint16_t kResolution{ 2048 };
	constexpr double kIntQ411ToDegrees{ 180.0 / (kResolution * kPi) };

	return static_cast<double>(value) * kIntQ411ToDegrees;
}

void LogAoADegreesFromQ411(int16_t q411Aoa) noexcept
{
	const double deg = IntQ411ToDegrees(q411Aoa);
	const int32_t mdeg = static_cast<int32_t>(deg >= 0.0 ? deg * 1000.0 + 0.5 : deg * 1000.0 - 0.5);

	const auto parts = Aliro::Uwb::Utils::SplitMilli(mdeg);
	LOG_INF("AoA [degrees]: %s%u.%03u", parts.mSign, static_cast<unsigned int>(parts.mInteger),
		static_cast<unsigned int>(parts.mFraction));
}

void PrintDiagnosticReport(const cherry_common_aoa_measurement *aoas, unsigned int nAoa) noexcept
{
	if (!aoas || nAoa == 0) {
		LOG_ERR("Invalid AoA measurements.");
		return;
	}

	for (unsigned int i = 0; i < nAoa; i++) {
		const auto &aoa = aoas[i];
		LOG_INF("AoA Q4.11 measurement: tdoa: %d, pdoa: %d, aoa: %d, fom: %d, type: %d", aoa.tdoa, aoa.pdoa,
			aoa.aoa, aoa.fom, aoa.type);

		LogAoADegreesFromQ411(aoa.aoa);
	}
}

void DefaultDiagnosticReportHandler(const cherry_common_diag_report *diagnostics) noexcept
{
	VerifyOrReturn(diagnostics, LOG_ERR("Invalid diagnostic report."));

	for (size_t i = 0; i < diagnostics->n_frame_report; i++) {
		const auto &frame = diagnostics->frame_report[i];
		if (frame.aoas && frame.n_aoa > 0) {
			PrintDiagnosticReport(frame.aoas, frame.n_aoa);
		}
	}
}

} // namespace
#endif // CONFIG_DOOR_LOCK_ALIRO_UWB_QM35_DIAGNOSTICS_DEFAULT_HANDLER

namespace Aliro::Uwb {

void UwbDiagnostics::Init(aliro_uwb_adapter *aliroCtx, OnDiagnosticReport onDiagnosticReport)
{
	mAliroCtx = aliroCtx;
	aliro_uwb_adapter_set_diagnostics(mAliroCtx, { .aoa = true, .rssi = true });
	mOnDiagnosticReport = onDiagnosticReport;

	SessionEventHub::Register(mSubscriber);
}

void UwbDiagnostics::OnSessionEvent(const aliro_uwb_session_event &event, const SessionContext &sessionCtx, void *ctx)
{
	VerifyOrReturn(ctx);

	if (event.type == ALIRO_UWB_SESSION_EVENT_TYPE_SESSION_DIAGNOSTIC_REPORT) {
#if CONFIG_DOOR_LOCK_ALIRO_UWB_QM35_DIAGNOSTICS_DEFAULT_HANDLER
		DefaultDiagnosticReportHandler(event.data.diagnostics);
#endif // CONFIG_DOOR_LOCK_ALIRO_UWB_QM35_DIAGNOSTICS_DEFAULT_HANDLER

		auto *diagnostics = static_cast<UwbDiagnostics *>(ctx);
		VerifyAndCall(diagnostics->mOnDiagnosticReport, event.data.diagnostics);
	}
}

} // namespace Aliro::Uwb
