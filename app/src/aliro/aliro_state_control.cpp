/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "aliro_state_control.h"

#include "aliro/init.h"
#include "reader.h"

#include <aliro/aliro.h>
#include <aliro/utils.h>

#include <cstdlib>

#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(aliro);

namespace {

bool IsProvisioningComplete()
{
	return DoorLock::Storage::Reader::IsIdentifierSet() && DoorLock::Storage::Reader::IsPrivateKeySet();
}

} // anonymous namespace

namespace DoorLock::AliroStateControl {

AliroError UpdateAliroState()
{
	if (IsProvisioningComplete()) {
		if (!IsAliroRunning()) {
			const int startRc = AliroStart();
			VerifyOrReturnStatus(startRc == EXIT_SUCCESS, ALIRO_ERROR_INTERNAL,
					     LOG_ERR("Failed to start Aliro: %d", startRc));
#ifdef CONFIG_DOOR_LOCK_BLE_UWB
		} else {
			ReturnErrorOnFailure(StartAliroAdvertising());
#endif // CONFIG_DOOR_LOCK_BLE_UWB
		}
		return ALIRO_NO_ERROR;
	}

	if (IsAliroRunning()) {
		const int rc = AliroStop();
		VerifyOrReturnStatus(rc == EXIT_SUCCESS, ALIRO_ERROR_INTERNAL, LOG_ERR("Failed to stop Aliro: %d", rc));
	}
	return ALIRO_NO_ERROR;
}

} // namespace DoorLock::AliroStateControl
