/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <aliro/interface.h>

#ifdef CONFIG_DOOR_LOCK_TIME_CONCEPT
#include <aliro/utils.h>
#include <time_utils/time_utils.h>
#endif // CONFIG_DOOR_LOCK_TIME_CONCEPT

#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(interface_access_document, CONFIG_DOOR_LOCK_APP_LOG_LEVEL);

namespace Aliro::Interface::AccessDocument {

std::optional<bool> VerifyValidityPeriod(const Time &validFrom, const Time &validUntil)
{
#ifdef CONFIG_DOOR_LOCK_TIME_CONCEPT
	const auto currentTimeOpt = DoorLock::TimeUtils::GetCurrentTime();
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
	LOG_WRN("Time concept is not supported");
	return std::nullopt;
#endif // CONFIG_DOOR_LOCK_TIME_CONCEPT
}

} // namespace Aliro::Interface::AccessDocument
