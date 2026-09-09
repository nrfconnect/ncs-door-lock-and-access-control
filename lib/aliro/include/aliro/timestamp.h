/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace Aliro {

/** @brief Length of an RFC 3339 timestamp. */
constexpr size_t kTimestampLength{ 20 };

/** @brief Timestamp encoded in RFC 3339 format. */
using Timestamp = std::array<uint8_t, kTimestampLength>;

} // namespace Aliro
