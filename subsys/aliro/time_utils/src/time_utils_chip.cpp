/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <time_utils/time_utils.h>

#include <app/server/Server.h>
#include <lib/support/TimeUtils.h>

#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(time_utils, CONFIG_DOOR_LOCK_ALIRO_TIME_UTILS_LOG_LEVEL);

namespace DoorLock::TimeUtils {

using namespace ::chip;

std::optional<Aliro::Time> GetCurrentTime()
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
		return Aliro::Time(year, month, dayOfMonth, hour, minute, second);
	}

	LOG_WRN("Wall clock time not available, falling back to Last Known Good Time");

	System::Clock::Seconds32 lastKnownGoodChipEpochTime{};
	err = Server::GetInstance().GetFabricTable().GetLastKnownGoodChipEpochTime(lastKnownGoodChipEpochTime);
	if (err != CHIP_NO_ERROR) {
		LOG_ERR("Failed to retrieve Last Known Good UTC Time");
		return std::nullopt;
	}

	ChipEpochToCalendarTime(lastKnownGoodChipEpochTime.count(), year, month, dayOfMonth, hour, minute, second);
	return Aliro::Time(year, month, dayOfMonth, hour, minute, second);
}

std::optional<uint32_t> GetCurrentUnixTime()
{
	System::Clock::Milliseconds64 currentUnixTimeMS;
	const auto err = System::SystemClock().GetClock_RealTimeMS(currentUnixTimeMS);

	if (err != CHIP_NO_ERROR) {
		LOG_ERR("Wall clock time not available");
		return std::nullopt;
	}

	return static_cast<uint32_t>(currentUnixTimeMS.count() / 1000);
}

} // namespace DoorLock::TimeUtils
