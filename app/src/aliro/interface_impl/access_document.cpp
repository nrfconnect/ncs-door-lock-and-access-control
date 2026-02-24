/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "aliro/interface.h"

#ifdef CONFIG_CHIP
#include <app/server/Server.h>
#include <lib/support/TimeUtils.h>
#endif // CONFIG_CHIP

#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(interface_access_document, CONFIG_DOOR_LOCK_APP_LOG_LEVEL);

namespace Aliro::Interface::AccessDocument {

namespace {

#ifdef CONFIG_CHIP

using namespace ::chip;

std::optional<Time> GetCurrentTime()
{
	uint16_t year{};
	uint8_t month{};
	uint8_t dayOfMonth{};
	uint8_t hour{};
	uint8_t minute{};
	uint8_t second{};

	System::Clock::Milliseconds64 currentUnixTimeMS;
	auto err = System::SystemClock().GetClock_RealTimeMS(currentUnixTimeMS);

	if (err == CHIP_NO_ERROR) {
		const auto currentUnixTime = std::chrono::duration_cast<System::Clock::Seconds32>(currentUnixTimeMS);
		SecondsSinceUnixEpochToCalendarTime(currentUnixTime.count(), year, month, dayOfMonth, hour, minute,
						    second);
		return Time(year, month, dayOfMonth, hour, minute, second);
	}

	LOG_WRN("Wall clock time not available, falling back to Last Known Good Time");

	System::Clock::Seconds32 lastKnownGoodChipEpochTime{};
	err = Server::GetInstance().GetFabricTable().GetLastKnownGoodChipEpochTime(lastKnownGoodChipEpochTime);
	if (err != CHIP_NO_ERROR) {
		LOG_ERR("Failed to retrieve Last Known Good UTC Time");
		return std::nullopt;
	}

	ChipEpochToCalendarTime(lastKnownGoodChipEpochTime.count(), year, month, dayOfMonth, hour, minute, second);
	return Time(year, month, dayOfMonth, hour, minute, second);
}

#endif // CONFIG_CHIP

} // namespace

std::optional<bool> VerifyValidityPeriod(const Time &validFrom, const Time &validUntil)
{
#ifdef CONFIG_CHIP

	const auto currentTimeOpt = GetCurrentTime();
	VerifyOrReturnValue(currentTimeOpt.has_value(), std::nullopt);

	const auto &currentTime = currentTimeOpt.value();

	LOG_DBG("Current time: %04d-%02d-%02d %02d:%02d:%02d", currentTime.mYear, currentTime.mMonth, currentTime.mDay,
		currentTime.mHour, currentTime.mMinute, currentTime.mSecond);
	LOG_DBG("validFrom   : %04d-%02d-%02d %02d:%02d:%02d", validFrom.mYear, validFrom.mMonth, validFrom.mDay,
		validFrom.mHour, validFrom.mMinute, validFrom.mSecond);
	LOG_DBG("validUntil  : %04d-%02d-%02d %02d:%02d:%02d", validUntil.mYear, validUntil.mMonth, validUntil.mDay,
		validUntil.mHour, validUntil.mMinute, validUntil.mSecond);

	const bool isWithinValidityPeriod = currentTime >= validFrom && currentTime <= validUntil;
	if (!isWithinValidityPeriod) {
		LOG_WRN("Current time is outside the Access Document validity period");
	}

	return isWithinValidityPeriod;
#else
	ARG_UNUSED(validFrom);
	ARG_UNUSED(validUntil);
	return std::nullopt;
#endif // CONFIG_CHIP
}

} // namespace Aliro::Interface::AccessDocument
