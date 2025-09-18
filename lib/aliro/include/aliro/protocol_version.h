/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

#pragma once

#include <cstdint>

/**
 * @brief Protocol version definitions for expedited phase.
 */
namespace Aliro {

/**
 * @brief The protocol version type.
 */
using ProtocolVersion = uint16_t;

static_assert(sizeof(ProtocolVersion) == sizeof(uint16_t),
	      "ProtocolVersion type must be 2 bytes, as implementation depends on it");

} // namespace Aliro
