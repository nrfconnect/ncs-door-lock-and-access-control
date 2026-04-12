/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "aliro/aliro_state_control.h"

#include "time_sync_delegate.h"

#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(app, CONFIG_CHIP_APP_LOG_LEVEL);

void TimeSyncDelegate::UTCTimeAvailabilityChanged(uint64_t)
{
	const auto ec = DoorLock::AliroStateControl::UpdateAliroState();
	VerifyOrReturn(ec == ALIRO_NO_ERROR, LOG_ERR("Failed to update Aliro state: %d", ec.ToInt()));
}
