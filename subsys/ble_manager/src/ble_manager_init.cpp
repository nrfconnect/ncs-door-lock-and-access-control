/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <doorlock/utils/utils.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/logging/log.h>
#include <zephyr/random/random.h>

#include <array>

LOG_MODULE_DECLARE(BleManager, CONFIG_DOOR_LOCK_BLE_MANAGER_LOG_LEVEL);

namespace DoorLock::BleManager {

int InitStack()
{
	// Random static address for the default identity must be set before bt_enable()
	// on nRF5340; updating the default identity afterwards is not possible.
	bt_addr_le_t address{};
	int err = sys_csrand_get(address.a.val, sizeof(address.a.val));
	VerifyOrReturnValue(err == 0, err, LOG_ERR("Cannot generate BLE random address"));

	address.type = BT_ADDR_LE_RANDOM;
	BT_ADDR_SET_STATIC(&address.a);

	int idOrError = bt_id_create(&address, nullptr);
	VerifyOrReturnValue(idOrError >= 0, idOrError, LOG_ERR("Cannot create new ID (error: %d)", idOrError));

	err = bt_enable(nullptr);
	VerifyOrReturnValue(err == 0 || err == -EALREADY, err, LOG_ERR("Cannot enable BLE stack (error: %d)", err));

#ifdef CONFIG_LOG
	std::array<char, BT_ADDR_LE_STR_LEN> addressStr{};
	idOrError = bt_addr_le_to_str(&address, addressStr.data(), addressStr.size());
	VerifyOrReturnValue(idOrError >= 0, idOrError,
			    LOG_ERR("Cannot convert address to string (error: %d)", idOrError));

	LOG_INF("Doorlock BLE address: %s", addressStr.data());
#endif // CONFIG_LOG

	return 0;
}

} // namespace DoorLock::BleManager
