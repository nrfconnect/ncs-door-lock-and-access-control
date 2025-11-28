/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

#pragma once

#include "aliro/types.h"

#include <optional>

namespace Aliro {

/**
 * @struct Time
 * @brief Represents a date and time with year, month, day, hour, minute, and second components.
 *
 * This struct provides a simple representation of calendar time and supports
 * comparison operations for ordering and equality checks.
 */
struct Time {
	/**
	 * @brief Constructs a Time object with the specified date and time components.
	 *
	 * @param year The year component (e.g., 2025).
	 * @param month The month component (1-12).
	 * @param day The day component (1-31).
	 * @param hour The hour component (0-23).
	 * @param minute The minute component (0-59).
	 * @param second The second component (0-60).
	 *
	 * @note Value 60 for second is valid for leap seconds per RFC 3339.
	 */
	explicit constexpr Time(int year, int month, int day, int hour, int minute, int second)
		: mYear(year), mMonth(month), mDay(day), mHour(hour), mMinute(minute), mSecond(second)
	{
	}

	/**
	 * @brief Less-than comparison operator.
	 *
	 * Compares two Time objects using lexicographic ordering
	 * of components: year, month, day, hour, minute, second.
	 *
	 * @param lhs The left-hand side Time object.
	 * @param rhs The right-hand side Time object.
	 * @return true if lhs is earlier than rhs, false otherwise.
	 */
	friend bool operator<(const Time &lhs, const Time &rhs)
	{
		if (lhs.mYear != rhs.mYear)
			return lhs.mYear < rhs.mYear;
		if (lhs.mMonth != rhs.mMonth)
			return lhs.mMonth < rhs.mMonth;
		if (lhs.mDay != rhs.mDay)
			return lhs.mDay < rhs.mDay;
		if (lhs.mHour != rhs.mHour)
			return lhs.mHour < rhs.mHour;
		if (lhs.mMinute != rhs.mMinute)
			return lhs.mMinute < rhs.mMinute;
		return lhs.mSecond < rhs.mSecond;
	}

	/**
	 * @brief Greater-than comparison operator.
	 *
	 * @param lhs The left-hand side Time object.
	 * @param rhs The right-hand side Time object.
	 * @return true if lhs is later than rhs, false otherwise.
	 */
	friend bool operator>(const Time &lhs, const Time &rhs) { return rhs < lhs; }

	/**
	 * @brief Greater-than or equal to comparison operator.
	 *
	 * @param lhs The left-hand side Time object.
	 * @param rhs The right-hand side Time object.
	 * @return true if lhs is later than or equal to rhs, false otherwise.
	 */
	friend bool operator>=(const Time &lhs, const Time &rhs) { return !(lhs < rhs); }

	/**
	 * @brief Less-than or equal to comparison operator.
	 *
	 * @param lhs The left-hand side Time object.
	 * @param rhs The right-hand side Time object.
	 * @return true if lhs is earlier than or equal to rhs, false otherwise.
	 */
	friend bool operator<=(const Time &lhs, const Time &rhs) { return !(rhs < lhs); }

	/**
	 * @brief Equality comparison operator.
	 *
	 * Checks if all time components (year, month, day, hour, minute, second)
	 * are equal between the two Time objects.
	 *
	 * @param lhs The left-hand side Time object.
	 * @param rhs The right-hand side Time object.
	 * @return true if all components are equal, false otherwise.
	 */
	friend bool operator==(const Time &lhs, const Time &rhs)
	{
		return lhs.mYear == rhs.mYear && lhs.mMonth == rhs.mMonth && lhs.mDay == rhs.mDay &&
		       lhs.mHour == rhs.mHour && lhs.mMinute == rhs.mMinute && lhs.mSecond == rhs.mSecond;
	}

	/**
	 * @brief Inequality comparison operator.
	 *
	 * @param lhs The left-hand side Time object.
	 * @param rhs The right-hand side Time object.
	 * @return true if lhs is not equal to rhs, false otherwise.
	 */
	friend bool operator!=(const Time &lhs, const Time &rhs) { return !(lhs == rhs); }

	/**
	 * @brief Creates a Time object from a Timestamp.
	 *
	 * Converts a Timestamp (RFC 3339 format) into a Time struct representation.
	 * The conversion may fail if the timestamp format is invalid.
	 *
	 * @param timestamp The timestamp to convert from.
	 * @return std::optional<Time> containing the Time if conversion succeeds,
	 *         or std::nullopt if the timestamp is invalid.
	 */
	static std::optional<Time> FromTimestamp(const Timestamp &timestamp)
	{
		return FromTimestamp(timestamp.data(), timestamp.size());
	}

	/**
	 * @brief Creates a Time object from a Timestamp.
	 *
	 * Converts a Timestamp (RFC 3339 format) into a Time struct representation.
	 * The conversion may fail if the timestamp format is invalid.
	 *
	 * @param timestamp The byte array to convert from.
	 * @param length The length of the byte array.
	 * @return std::optional<Time> containing the Time if conversion succeeds,
	 *         or std::nullopt if the timestamp is invalid.
	 */
	static std::optional<Time> FromTimestamp(const uint8_t *timestamp, size_t length);

	/** @brief The year component (e.g., 2025). */
	int mYear;
	/** @brief The month component (1-12). */
	int mMonth;
	/** @brief The day component (1-31). */
	int mDay;
	/** @brief The hour component (0-23). */
	int mHour;
	/** @brief The minute component (0-59). */
	int mMinute;
	/** @brief The second component (0-60). */
	int mSecond;
};

} // namespace Aliro
