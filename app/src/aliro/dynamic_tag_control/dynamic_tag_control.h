/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

namespace DoorLock::DynamicTagControl {

/**
 * @brief Start the Dynamic Tag Controller with a periodic update
 *
 * The periodic update is scheduled with the interval configured by the
 * DOOR_LOCK_DYNAMIC_TAG_CONTROL_EXPIRY_DURATION_S configuration option.
 *
 * @return 0 on success, negative error code otherwise.
 */
int Start();

/**
 * @brief Stop Dynamic Tag updater.
 *
 * Cancels any pending periodic updates.
 */
void Stop();

} // namespace DoorLock::DynamicTagControl
