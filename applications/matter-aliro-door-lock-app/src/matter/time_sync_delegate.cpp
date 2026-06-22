/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "time_sync_delegate.h"

#ifdef CONFIG_DOOR_LOCK_ALIRO_BLE_SERVICE_DYNAMIC_TAG
#include <aliro_service/aliro_service.h>
#endif // CONFIG_DOOR_LOCK_ALIRO_BLE_SERVICE_DYNAMIC_TAG

void TimeSyncDelegate::UTCTimeAvailabilityChanged(uint64_t)
{
#ifdef CONFIG_DOOR_LOCK_ALIRO_BLE_SERVICE_DYNAMIC_TAG
	DoorLock::AliroService::RefreshAdvertising();
#endif // CONFIG_DOOR_LOCK_ALIRO_BLE_SERVICE_DYNAMIC_TAG
}
