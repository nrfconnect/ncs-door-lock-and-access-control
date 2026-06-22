/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "utils.h"

#include <doorlock/utils/utils.h>

#include <zephyr/bluetooth/addr.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/hci_vs.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/random/random.h>

#include <cerrno>
#include <cstring>

LOG_MODULE_DECLARE(AliroService, CONFIG_DOOR_LOCK_ALIRO_BLE_SERVICE_LOG_LEVEL);

static_assert(CONFIG_DOOR_LOCK_ALIRO_BLE_SERVICE_BT_ID != BT_ID_DEFAULT,
	      "Aliro Bluetooth identity must not use BT_ID_DEFAULT");
static_assert(CONFIG_BT_ID_MAX > CONFIG_DOOR_LOCK_ALIRO_BLE_SERVICE_BT_ID,
	      "CONFIG_BT_ID_MAX must be greater than the configured Aliro Bluetooth identity");

namespace DoorLock::AliroService::Utils {

namespace {

int CreateRandomStaticAddress(bt_addr_le_t &addr)
{
	const int err = sys_csrand_get(addr.a.val, sizeof(addr.a.val));
	VerifyOrReturnValue(err == 0, err, LOG_ERR("Failed to generate random address: %d", err));

	addr.type = BT_ADDR_LE_RANDOM;
	BT_ADDR_SET_STATIC(&addr.a);

	return 0;
}

} // namespace

int CreateBtId(Aliro::BleTypes::BleAddress &address)
{
	size_t count{ CONFIG_BT_ID_MAX };
	bt_id_get(nullptr, &count);
	VerifyOrReturnValue(count < kAliroBtId + 1, -EALREADY, LOG_ERR("Aliro Bluetooth ID already exists"));

	bt_addr_le_t addr;

	for (size_t i = count; i <= kAliroBtId; i++) {
		int err = CreateRandomStaticAddress(addr);
		VerifyOrReturnValue(err == 0, err);
		err = bt_id_create(&addr, nullptr);
		VerifyOrReturnValue(err >= 0, err, LOG_ERR("Failed to create BT ID: %d", err));
	}

	std::memcpy(address.data(), addr.a.val, sizeof(address));

#ifdef CONFIG_LOG
	char addressStr[BT_ADDR_LE_STR_LEN];
	const int err = bt_addr_le_to_str(&addr, addressStr, sizeof(addressStr));
	VerifyOrReturnValue(err >= 0, err, LOG_ERR("Cannot convert address to string: %d", err));
	LOG_INF("Aliro BLE address: %s", addressStr);
#endif // CONFIG_LOG

	return 0;
}

#if defined(CONFIG_SOC_NRF5340_CPUAPP) || defined(CONFIG_BT_CTLR_TX_PWR_DYNAMIC_CONTROL)

int GetTxPower(Aliro::BleTypes::TxPowerLevel &txPowerLevel)
{
	constexpr uint16_t kHandle{ 0 };
	constexpr uint8_t kHandleType{ BT_HCI_VS_LL_HANDLE_TYPE_ADV };

	net_buf *cmdBuff = bt_hci_cmd_alloc(K_FOREVER);
	VerifyOrReturnValue(cmdBuff, -ENOMEM, LOG_ERR("Cannot allocate memory for HCI command"));

	auto *cmd = static_cast<bt_hci_cp_vs_read_tx_power_level *>(
		net_buf_add(cmdBuff, sizeof(bt_hci_cp_vs_read_tx_power_level)));
	VerifyOrReturnValue(cmd, -EIO, LOG_ERR("Cannot add data to the net buffer for HCI command"));

	cmd->handle = kHandle;
	cmd->handle_type = kHandleType;

	net_buf *respBuff{};
	int error = bt_hci_cmd_send_sync(BT_HCI_OP_VS_READ_TX_POWER_LEVEL, cmdBuff, &respBuff);
	VerifyOrReturnValue(error == 0, error, LOG_ERR("Failed to read Tx power level (error: %d)", error));
	VerifyOrReturnValue(respBuff->len >= sizeof(bt_hci_rp_vs_read_tx_power_level), -EIO,
			    LOG_ERR("Invalid response buffer size"));

	bt_hci_rp_vs_read_tx_power_level resp{};
	std::memcpy(&resp, respBuff->data, sizeof(bt_hci_rp_vs_read_tx_power_level));
	txPowerLevel = resp.tx_power_level;

	net_buf_unref(respBuff);

	return 0;
}

#endif // CONFIG_SOC_NRF5340_CPUAPP || CONFIG_BT_CTLR_TX_PWR_DYNAMIC_CONTROL

} // namespace DoorLock::AliroService::Utils
