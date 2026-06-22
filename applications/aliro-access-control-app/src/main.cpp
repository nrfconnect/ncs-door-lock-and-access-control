/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "aliro/init.h"
#include "aliro/lock_sim_instance.h"

#include <crypto_utils/crypto_utils.h>
#include <doorlock/utils/utils.h>
#include <zephyr/logging/log.h>

#include <cstdlib>

#ifdef CONFIG_DOOR_LOCK_DFU_SMP_SERVICE
#include <dfu_smp_service/dfu_smp_service.h>
#include <zephyr/bluetooth/gap.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#endif // CONFIG_DOOR_LOCK_DFU_SMP_SERVICE

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

namespace {

enum class AdvertisingPriority : uint8_t {
	DFU_SMP,
	NUS,
};

#ifdef CONFIG_DOOR_LOCK_DFU_SMP_SERVICE

#if DT_HAS_ALIAS(dfu_smp_button)

gpio_callback sDfuSmpButtonCb{};

int InitDfuSmpButtonGpio()
{
	constexpr gpio_dt_spec button = GPIO_DT_SPEC_GET(DT_ALIAS(dfu_smp_button), gpios);

	VerifyOrReturnValue(device_is_ready(button.port), -ENODEV, LOG_ERR("SMP button GPIO device not ready"));

	int err = gpio_pin_configure_dt(&button, GPIO_INPUT);
	VerifyOrReturnValue(err == 0, err, LOG_ERR("Failed to configure SMP button GPIO"));

	err = gpio_pin_interrupt_configure_dt(&button, GPIO_INT_EDGE_TO_ACTIVE);
	VerifyOrReturnValue(err == 0, err, LOG_ERR("Failed to configure SMP button interrupt"));

	gpio_init_callback(
		&sDfuSmpButtonCb, [](const device *, gpio_callback *, uint32_t) { DoorLock::DfuSmpService::Toggle(); },
		BIT(button.pin));

	err = gpio_add_callback(button.port, &sDfuSmpButtonCb);
	VerifyOrReturnValue(err == 0, err, LOG_ERR("Failed to add SMP button callback"));

	return 0;
}

#endif // DT_HAS_ALIAS(dfu_smp_button)

int InitDfuSmpButton()
{
#if DT_HAS_ALIAS(dfu_smp_button)
	return InitDfuSmpButtonGpio();
#else
	return 0;
#endif
}

#endif // CONFIG_DOOR_LOCK_DFU_SMP_SERVICE

} // namespace

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

#ifdef CONFIG_DOOR_LOCK_DFU_SMP_SERVICE

	DoorLock::DfuSmpService::Init(DoorLock::Utils::ToUnderlying(AdvertisingPriority::DFU_SMP),
				      BT_GAP_ADV_FAST_INT_MIN_2, BT_GAP_ADV_FAST_INT_MAX_2);

	int dfuSmpErr = InitDfuSmpButton();
	VerifyOrDie(dfuSmpErr == 0, "Failed to initialize DFU SMP button");

	DoorLock::DfuSmpService::ConfirmNewImage();

#endif // CONFIG_DOOR_LOCK_DFU_SMP_SERVICE

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
	nusErr = DoorLock::NUSService::Start(DoorLock::Utils::ToUnderlying(AdvertisingPriority::NUS),
					     BT_GAP_ADV_FAST_INT_MIN_2, BT_GAP_ADV_FAST_INT_MAX_2);
	VerifyOrDie(nusErr == 0, "Failed to start NUS service");

#endif // CONFIG_DOOR_LOCK_NUS_SERVICE

	LOG_INF("Application started");

	return EXIT_SUCCESS;
}
