/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include <aliro/time.h>

#include <cstdint>
#include <optional>

namespace DoorLock::TimeUtils {

/**
 * @brief Get the current time in Aliro format.
 *
 * @return The current time when available, or std::nullopt if it's not available.
 */
std::optional<Aliro::Time> GetCurrentTime();

/**
 * @brief Get the current Unix time in seconds.
 *
 * @return Current Unix epoch time as uint32_t, or std::nullopt if it's not available.
 */
std::optional<uint32_t> GetCurrentUnixTime();

} // namespace DoorLock::TimeUtils
