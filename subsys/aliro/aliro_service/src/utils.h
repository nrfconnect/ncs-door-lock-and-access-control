/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include <aliro/ble_types.h>
#include <zephyr/bluetooth/addr.h>

namespace DoorLock::AliroService::Utils {

static constexpr uint8_t kAliroBtId{ CONFIG_DOOR_LOCK_ALIRO_BLE_SERVICE_BT_ID };

/**
 * Creates the Bluetooth identity used by the Aliro service.
 *
 * @param address Bluetooth address assigned to the Aliro identity.
 * @return 0 on success, or a negative error code on failure.
 */
int CreateBtId(Aliro::BleTypes::BleAddress &address);

#if defined(CONFIG_SOC_NRF5340_CPUAPP) || defined(CONFIG_BT_CTLR_TX_PWR_DYNAMIC_CONTROL)

/**
 * Gets the current Bluetooth LE transmit power level used for Aliro advertising.
 *
 * @param txPowerLevel Transmit power level reported by the Bluetooth controller.
 * @return 0 on success, or a negative error code on failure.
 */
int GetTxPower(Aliro::BleTypes::TxPowerLevel &txPowerLevel);

#else

/**
 * Gets the configured Bluetooth LE transmit power level used for Aliro advertising.
 *
 * @param txPowerLevel Transmit power level from the static controller configuration.
 * @return 0 on success.
 */
inline int GetTxPower(Aliro::BleTypes::TxPowerLevel &txPowerLevel)
{
	txPowerLevel = static_cast<Aliro::BleTypes::TxPowerLevel>(CONFIG_BT_CTLR_TX_PWR_DBM);
	return 0;
}

#endif // CONFIG_SOC_NRF5340_CPUAPP || CONFIG_BT_CTLR_TX_PWR_DYNAMIC_CONTROL

} // namespace DoorLock::AliroService::Utils
