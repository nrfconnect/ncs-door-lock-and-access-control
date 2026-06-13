/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include <cstdint>

namespace DoorLock::DfuSmpService::Advertising {

/**
 * @brief Insert DFU SMP advertising request into the advertising arbiter.
 *
 * @param priority Advertising priority.
 * @param minInterval Advertising interval minimum.
 * @param maxInterval Advertising interval maximum.
 * @return 0 on success, or a negative error code on failure.
 */
int InsertRequest(uint8_t priority, uint16_t minInterval, uint16_t maxInterval);

/**
 * @brief Cancel DFU SMP advertising request from the advertising arbiter.
 */
void CancelRequest();

/**
 * @return True if the DFU SMP advertising request is active in the arbiter.
 */
bool IsActive();

} // namespace DoorLock::DfuSmpService::Advertising
