/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include <aliro/errors.h>

namespace Aliro::Access::Indicator {

/**
 * @brief Initializes a access decision indicator (e.g LED).
 * This function sets up the access decision indicator to be used
 * when an access decision is made.
 *
 * @return ALIRO_NO_ERROR on success, or an error code on failure.
 */
AliroError InitAccessDecisionIndicator();

/**
 * @brief Signals that access has been granted.
 * This function activates the access decision indicator (e.g LED)
 * to indicate that access has been granted.
 * It also schedules a work to reset the indicator state after a delay.
 */
void SignalAccessGranted();

} // namespace Aliro::Access::Indicator
