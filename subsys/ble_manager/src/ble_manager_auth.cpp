/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <doorlock/utils/utils.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/hci_types.h>
#include <zephyr/logging/log.h>

#include <cstdint>

LOG_MODULE_DECLARE(BleManager, CONFIG_DOOR_LOCK_BLE_MANAGER_LOG_LEVEL);

namespace DoorLock::BleManager {

namespace {

bool IsDefaultIdentity(bt_conn *conn)
{
	VerifyOrReturnValue(conn, false);

	bt_conn_info info{};
	const int err = bt_conn_get_info(conn, &info);
	VerifyOrReturnValue(err == 0, false, LOG_ERR("bt_conn_get_info failed (%d)", err));

	return info.id == BT_ID_DEFAULT;
}

char *AddressString(const bt_conn *conn)
{
#ifdef CONFIG_LOG
	static char addr[BT_ADDR_LE_STR_LEN];
	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));
	return addr;
#else
	ARG_UNUSED(conn);
	return nullptr;
#endif
}

void AuthPasskeyDisplay(bt_conn *conn, unsigned int passkey)
{
	VerifyOrReturn(IsDefaultIdentity(conn), LOG_DBG("Ignoring passkey display for non-default identity"));

	LOG_INF("PROVIDE THE FOLLOWING CODE IN YOUR MOBILE APP: %u", passkey);
}

void AuthCancel(bt_conn *conn)
{
	VerifyOrReturn(IsDefaultIdentity(conn), LOG_DBG("Ignoring auth cancel for non-default identity"));

	LOG_INF("BLE pairing cancelled: %s", AddressString(conn));
	bt_conn_disconnect(conn, BT_HCI_ERR_REMOTE_USER_TERM_CONN);
}

#ifdef CONFIG_DOOR_LOCK_BLE_MANAGER_AUTH_APP_PASSKEY_SUPPORT

uint32_t AuthAppPasskey(bt_conn *conn)
{
	VerifyOrReturnValue(IsDefaultIdentity(conn), BT_PASSKEY_RAND,
			    LOG_DBG("Ignoring app passkey for non-default identity"));

	return CONFIG_DOOR_LOCK_BLE_MANAGER_AUTH_APP_PASSKEY;
}

#endif // CONFIG_DOOR_LOCK_BLE_MANAGER_AUTH_APP_PASSKEY_SUPPORT

void PairingComplete(bt_conn *conn, bool bonded)
{
	VerifyOrReturn(IsDefaultIdentity(conn), LOG_DBG("Ignoring pairing complete for non-default identity"));

	LOG_INF("BLE pairing completed: %s, bonded: %d", AddressString(conn), bonded);
}

void PairingFailed(bt_conn *conn, bt_security_err reason)
{
	VerifyOrReturn(IsDefaultIdentity(conn), LOG_DBG("Ignoring pairing failed for non-default identity"));

	LOG_INF("BLE pairing failed to %s: reason %u", AddressString(conn), static_cast<unsigned int>(reason));
}

#if defined(CONFIG_BT_SMP)

void ConnSecurityChanged(bt_conn *conn, bt_security_t level, bt_security_err err)
{
	VerifyOrReturn(IsDefaultIdentity(conn), LOG_DBG("Ignoring BLE security change for non-default identity"));

	VerifyOrReturn(err == BT_SECURITY_ERR_SUCCESS,
		       LOG_WRN("Security failed (error: %d, conn: %p)", static_cast<int>(err), conn));

	LOG_INF("BLE Security changed: %s level %d", AddressString(conn), static_cast<int>(level));
}

#endif // CONFIG_BT_SMP

bt_conn_auth_cb sConnAuthCallbacks{
	.passkey_display = AuthPasskeyDisplay,
	.cancel = AuthCancel,
#ifdef CONFIG_DOOR_LOCK_BLE_MANAGER_AUTH_APP_PASSKEY_SUPPORT
	.app_passkey = AuthAppPasskey,
#endif // CONFIG_DOOR_LOCK_BLE_MANAGER_AUTH_APP_PASSKEY_SUPPORT
};

bt_conn_auth_info_cb sConnAuthInfoCallbacks{
	.pairing_complete = PairingComplete,
	.pairing_failed = PairingFailed,
};

#if defined(CONFIG_BT_SMP)

bt_conn_cb sConnCallbacks{
	.security_changed = ConnSecurityChanged,
};

#endif // CONFIG_BT_SMP

} // namespace

int InitAuthentication()
{
	int err = bt_conn_auth_cb_register(&sConnAuthCallbacks);
	VerifyOrReturnValue(err == 0, err, LOG_ERR("Failed to register SMP auth callbacks"));

	err = bt_conn_auth_info_cb_register(&sConnAuthInfoCallbacks);
	VerifyOrReturnValue(err == 0, err, LOG_ERR("Failed to register SMP auth info callbacks"));

#if defined(CONFIG_BT_SMP)
	err = bt_conn_cb_register(&sConnCallbacks);
	VerifyOrReturnValue(err == 0, err, LOG_ERR("Failed to register SMP connection callbacks"));
#endif // CONFIG_BT_SMP

	return 0;
}

} // namespace DoorLock::BleManager
