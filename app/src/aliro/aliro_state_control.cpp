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
#include <aliro_workqueue/aliro_workqueue.h>

#include <cstdlib>

#include <zephyr/logging/log.h>

LOG_MODULE_DECLARE(aliro, CONFIG_DOOR_LOCK_APP_LOG_LEVEL);

namespace {

bool IsProvisioningComplete()
{
	return DoorLock::Storage::Reader::IsIdentifierSet() && DoorLock::Storage::Reader::IsPrivateKeySet();
}

void UpdateAliroStateWorkHandler(k_work *)
{
	if (IsProvisioningComplete()) {
		if (!IsAliroRunning()) {
			const auto rc = AliroStart();
			VerifyOrReturn(rc == EXIT_SUCCESS, LOG_ERR("Failed to start Aliro: %d", rc));
		}
#ifdef CONFIG_DOOR_LOCK_BLE_UWB
		else {
			VerifyOrReturn(StartAliroAdvertising() == ALIRO_NO_ERROR,
				       LOG_ERR("Failed to start Aliro advertising"));
		}
#endif // CONFIG_DOOR_LOCK_BLE_UWB
	} else if (IsAliroRunning()) {
		const auto rc = AliroStop();
		VerifyOrReturn(rc == EXIT_SUCCESS, LOG_ERR("Failed to stop Aliro: %d", rc));
	}
}

K_WORK_DEFINE(sUpdateAliroStateWork, UpdateAliroStateWorkHandler);

} // anonymous namespace

namespace DoorLock::AliroStateControl {

AliroError UpdateAliroState()
{
	const auto rc = AliroWorkqueueSubmit(&sUpdateAliroStateWork);
	VerifyOrReturnStatus(rc >= 0, ALIRO_ERROR_INTERNAL, LOG_ERR("Failed to submit update Aliro state work"));
	return ALIRO_NO_ERROR;
}

} // namespace DoorLock::AliroStateControl
