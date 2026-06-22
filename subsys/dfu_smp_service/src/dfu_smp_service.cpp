/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <dfu_smp_service/dfu_smp_service.h>

#include "advertising.h"
#include "callbacks.h"

#include <bluetooth/services/dfu_smp.h>
#include <doorlock/utils/utils.h>

#include <zephyr/dfu/mcuboot.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <tuple>

LOG_MODULE_REGISTER(DfuSmpService, CONFIG_DOOR_LOCK_DFU_SMP_SERVICE_LOG_LEVEL);

namespace {

k_work sWork;
uint8_t sPriority;
uint16_t sMinInterval;
uint16_t sMaxInterval;
bool sIsInitialized;

} // namespace

namespace DoorLock::DfuSmpService {

void Toggle()
{
	VerifyOrReturn(sIsInitialized, LOG_ERR("DFU SMP module not initialized"));
	std::ignore = k_work_submit(&sWork);
}

bool IsSmpEnabled()
{
	return Advertising::IsActive();
}

void Init(uint8_t priority, uint16_t minInterval, uint16_t maxInterval)
{
	sPriority = priority;
	sMinInterval = minInterval;
	sMaxInterval = maxInterval;

	Callbacks::Init();

	k_work_init(&sWork, []([[maybe_unused]] k_work *) {
		if (Advertising::IsActive()) {
			Advertising::CancelRequest();
		} else {
			const int err = Advertising::InsertRequest(sPriority, sMinInterval, sMaxInterval);
			VerifyOrReturn(err == 0, LOG_ERR("DFU SMP advertising request failed (rc %d)", err));
		}
	});

	sIsInitialized = true;

	LOG_DBG("DFU SMP module initialized");
}

void ConfirmNewImage()
{
	// Check if the image is run in the REVERT mode and eventually
	// confirm it to prevent reverting on the next boot.
	VerifyOrReturn(mcuboot_swap_type() == BOOT_SWAP_TYPE_REVERT);

	if (boot_write_img_confirmed()) {
		LOG_ERR("Failed to confirm firmware image, it will be reverted on the next boot");
	} else {
		LOG_DBG("New firmware image confirmed");
	}
}

} // namespace DoorLock::DfuSmpService
