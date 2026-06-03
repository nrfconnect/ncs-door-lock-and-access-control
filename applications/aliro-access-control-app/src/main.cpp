/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "aliro/init.h"
#include "aliro/lock_sim_instance.h"

#include "aliro/utils.h"

#include <crypto_utils/crypto_utils.h>
#include <zephyr/logging/log.h>

#include <cstdlib>

#ifdef CONFIG_DOOR_LOCK_DFU_BLE_SMP
#include "dfu_smp_manager.h"
#endif // CONFIG_DOOR_LOCK_DFU_BLE_SMP

#ifdef CONFIG_DOOR_LOCK_NUS_SERVICE
#include <nus_service/nus_service.h>
#include <zephyr/bluetooth/gap.h>
#endif // CONFIG_DOOR_LOCK_NUS_SERVICE

#ifdef CONFIG_DOOR_LOCK_BLE_UWB
#include "access_manager.h"
#include "uwb_impl.h"
#include <aliro/aliro.h>
#endif // CONFIG_DOOR_LOCK_BLE_UWB

LOG_MODULE_REGISTER(door_lock_app, CONFIG_DOOR_LOCK_APP_LOG_LEVEL);

enum class AdvertisingPriority : uint8_t {
	DFU_SMP,
	NUS,
};

int main()
{
	auto error = DoorLock::CryptoUtils::Init();
	VerifyOrDie(error == ALIRO_NO_ERROR, "Failed to initialize crypto utils");

#ifdef CONFIG_DOOR_LOCK_BLE_UWB

	constexpr Aliro::Uwb::UltraWideBand::Callbacks uwbCallbacks{
		.mRangingData =
			[](Aliro::Uwb::UltraWideBand::SessionContextHandle sessionContext,
			   const Aliro::UwbRangingData &uwbData) {
				Aliro::AccessManagerInstance().HandleRangingSessionData(sessionContext, uwbData);
			},
		.mRangingSessionStateChanged =
			[](Aliro::Uwb::UltraWideBand::SessionContextHandle sessionContext,
			   Aliro::RangingSessionState state) {
				Aliro::AccessManagerInstance().HandleRangingSessionStateChanged(sessionContext, state);
			},
		.mBleMessageTransmit =
			+[](Aliro::Uwb::UltraWideBand::SessionContextHandle sessionContext, const uint8_t *data,
			    size_t length) {
				Aliro::AliroStack::Instance().SendBleMessage(sessionContext, data, length);
			},
	};

	int uwbError = Aliro::Uwb::UltraWideBandInstance().Init(uwbCallbacks);
	if (uwbError == -ENOSYS) {
		LOG_INF("UWB is not implemented");
	} else if (uwbError != 0) {
		LOG_ERR("Failed to initialize UWB module: %d", uwbError);
	} else if (uwbError == 0) {
		auto *fwVersion = Aliro::Uwb::UltraWideBandInstance().GetFirmwareVersion();
		LOG_INF("UWB FW: %s", fwVersion ? fwVersion : "N/A");
	}

#endif // CONFIG_DOOR_LOCK_BLE_UWB

	int err = AliroInit();
	VerifyOrDie(err == EXIT_SUCCESS, "Failed to initialize Aliro");

#ifdef CONFIG_DOOR_LOCK_DFU_BLE_SMP

	Aliro::Dfu::SmpManager::Instance().Init();
	Aliro::Dfu::SmpManager::Instance().ConfirmNewImage();

#endif // CONFIG_DOOR_LOCK_DFU_BLE_SMP

#ifdef CONFIG_DOOR_LOCK_NUS_SERVICE

	DoorLock::NUSService::RegisterCommand(
		"Unlock",
		[](void *context) {
			LOG_INF("Unlock command received");
			Aliro::LockSimInstance().Unlock(Aliro::OperationSource::Unspecified);
		},
		nullptr);

	DoorLock::NUSService::RegisterCommand(
		"Lock",
		[](void *context) {
			LOG_INF("Lock command received");
			Aliro::LockSimInstance().Lock(Aliro::OperationSource::Unspecified);
		},
		nullptr);

	int nusErr = DoorLock::NUSService::Init();
	VerifyOrDie(nusErr == 0, "Failed to initialize NUS service");
	nusErr = DoorLock::NUSService::Start(ToUnderlying(AdvertisingPriority::NUS), BT_GAP_ADV_FAST_INT_MIN_2,
					     BT_GAP_ADV_FAST_INT_MAX_2);
	VerifyOrDie(nusErr == 0, "Failed to start NUS service");

#endif // CONFIG_DOOR_LOCK_NUS_SERVICE

	LOG_INF("Application started");

	return EXIT_SUCCESS;
}
