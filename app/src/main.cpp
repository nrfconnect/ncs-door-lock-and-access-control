/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifdef CONFIG_CHIP
#include "matter/init.h"
#else // CONFIG_CHIP
#include "aliro/init.h"
#endif // CONFIG_CHIP

#include "aliro/utils.h"

#include <crypto/utils.h>
#include <zephyr/logging/log.h>

#include <cstdlib>

#ifdef CONFIG_DOOR_LOCK_DFU_BLE_SMP
#include "dfu_smp_manager.h"
#endif // CONFIG_DOOR_LOCK_DFU_BLE_SMP

#ifdef CONFIG_DOOR_LOCK_BLE_NUS

#ifdef CONFIG_ACCESS_DECISION_INDICATOR
#include "aliro/platform/access_decision_indicator/access_decision_indicator.h"
#endif // CONFIG_ACCESS_DECISION_INDICATOR

#include "bt_nus/bt_nus.h"
#endif // CONFIG_DOOR_LOCK_BLE_NUS

#ifdef CONFIG_DOOR_LOCK_BLE_UWB
#include "access_manager.h"
#include "uwb_impl.h"
#endif // CONFIG_DOOR_LOCK_BLE_UWB

#ifdef CONFIG_CHIP
LOG_MODULE_REGISTER(app, CONFIG_CHIP_APP_LOG_LEVEL);
#else // CONFIG_CHIP
LOG_MODULE_REGISTER(door_lock_app, CONFIG_DOOR_LOCK_APP_LOG_LEVEL);
#endif // CONFIG_CHIP

int main()
{
	auto error = DoorLock::Crypto::Init();
	VerifyOrDie(error == ALIRO_NO_ERROR, "Failed to initialize Aliro crypto");

#ifdef CONFIG_DOOR_LOCK_BLE_UWB

	constexpr Aliro::Uwb::UltraWideBandImpl::Callbacks uwbCallbacks{
		.mRangingData =
			[](Aliro::Uwb::UltraWideBandImpl::SessionContextHandle sessionContext,
			   const Aliro::UwbRangingData &uwbData) {
				Aliro::AccessManagerInstance().HandleRangingSessionData(sessionContext, uwbData);
			},
		.mRangingSessionStateChanged =
			[](Aliro::Uwb::UltraWideBandImpl::SessionContextHandle sessionContext,
			   Aliro::RangingSessionState state) {
				Aliro::AccessManagerInstance().HandleRangingSessionStateChanged(sessionContext, state);
			}
	};

	error = Aliro::Uwb::UltraWideBandImpl::Instance().Init(uwbCallbacks);
	if (error == ALIRO_ERROR_NOT_IMPLEMENTED) {
		LOG_INF("UWB is not implemented");
	} else if (error != ALIRO_NO_ERROR) {
		LOG_ERR("Failed to initialize UWB module: %d", error.ToInt());
	}

#endif // CONFIG_DOOR_LOCK_BLE_UWB

#ifdef CONFIG_CHIP

	int err = StartMatter();
	VerifyOrDie(err == EXIT_SUCCESS, "Failed to start Matter");

#else // CONFIG_CHIP

	int err = AliroInit();
	VerifyOrDie(err == EXIT_SUCCESS, "Failed to initialize Aliro");

#ifdef CONFIG_DOOR_LOCK_DFU_BLE_SMP

	Aliro::Dfu::SmpManager::Instance().Init();
	Aliro::Dfu::SmpManager::Instance().ConfirmNewImage();

#endif // CONFIG_DOOR_LOCK_DFU_BLE_SMP

#ifdef CONFIG_DOOR_LOCK_BLE_NUS

	Aliro::BtNus::NUSService::Instance().RegisterCommand(
		"Unlock", strlen("Unlock"),
		[](void *context) {
			LOG_INF("Unlock command received");

#ifdef CONFIG_ACCESS_DECISION_INDICATOR
			Aliro::Access::Indicator::SignalAccessGranted();
#endif // CONFIG_ACCESS_DECISION_INDICATOR
		},
		nullptr);

	AliroError nusErr = Aliro::BtNus::NUSService::Instance().Start();
	VerifyOrDie(nusErr == ALIRO_NO_ERROR, "Failed to start NUS service");

#endif // CONFIG_DOOR_LOCK_BLE_NUS

	LOG_INF("Application started");

#endif // CONFIG_CHIP

	return EXIT_SUCCESS;
}
