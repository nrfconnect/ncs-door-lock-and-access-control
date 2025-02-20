/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include <zephyr/sys/util.h>

#include <cstddef>
#include <cstdint>

/**
 * @brief Protocol version definitions for expedited phase.
 */
namespace Aliro {
namespace ProtocolVersions {

using Type = uint16_t;

/**
 * @brief An array containing supported protocol versions for the expedited phase.
 *
 * This array lists all the protocol versions that are supported in the expedited phase.
 * Each version is represented as a 16-bit unsigned integer. This array may be extended
 * with other supported versions.
 * NOTE: The array of supported versions MUST be ordered from highest to lowest.
 */
static constexpr Type kSupportedVersions[] = { 0x0100 };

/**
 * @brief The number of supported protocol versions in the expedited phase.
 *
 * This constant represents the total number of entries in the kExpeditedPhaseProtocolVersions array.
 */
static constexpr size_t kNumberOfSupported{ ARRAY_SIZE(kSupportedVersions) };

} // namespace ProtocolVersions
} // namespace Aliro
