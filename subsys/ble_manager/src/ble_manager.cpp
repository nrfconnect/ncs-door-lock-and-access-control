/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <doorlock/utils/utils.h>

#include <zephyr/init.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(BleManager, CONFIG_DOOR_LOCK_BLE_MANAGER_LOG_LEVEL);

namespace DoorLock::BleManager {

#ifdef CONFIG_DOOR_LOCK_BLE_MANAGER_INIT
int InitStack();
#endif // CONFIG_DOOR_LOCK_BLE_MANAGER_INIT

#ifdef CONFIG_DOOR_LOCK_BLE_MANAGER_AUTH
int InitAuthentication();
#endif // CONFIG_DOOR_LOCK_BLE_MANAGER_AUTH

namespace {

int Init(void)
{
#ifdef CONFIG_DOOR_LOCK_BLE_MANAGER_AUTH
	const int err = InitAuthentication();
	VerifyOrReturnValue(err == 0, err, LOG_ERR("Bluetooth authentication initialization failed"));
#endif // CONFIG_DOOR_LOCK_BLE_MANAGER_AUTH

#ifdef CONFIG_DOOR_LOCK_BLE_MANAGER_INIT
	const int stackErr = InitStack();
	VerifyOrReturnValue(stackErr == 0, stackErr, LOG_ERR("Bluetooth stack initialization failed"));
#endif // CONFIG_DOOR_LOCK_BLE_MANAGER_INIT

	return 0;
}

SYS_INIT(Init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);

} // namespace

} // namespace DoorLock::BleManager
