/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include <cstdint>

namespace DoorLock::DfuSmpService {

/**
 * @brief Initialize the DFU SMP service.
 *
 * @param priority Advertising priority.
 * @param minInterval Advertising interval minimum.
 * @param maxInterval Advertising interval maximum.
 */
void Init(uint8_t priority, uint16_t minInterval, uint16_t maxInterval);

/**
 * @brief Toggle DFU SMP advertising.
 */
void Toggle();

/**
 * @brief Check whether DFU SMP advertising is enabled.
 *
 * @return true if DFU SMP advertising is enabled, false otherwise.
 */
bool IsSmpEnabled();

/**
 * @brief Confirm a newly booted image when it is pending revert.
 */
void ConfirmNewImage();

} // namespace DoorLock::DfuSmpService
