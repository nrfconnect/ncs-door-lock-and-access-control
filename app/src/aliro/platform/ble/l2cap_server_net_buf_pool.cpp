/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "transport/ble/l2cap_server_net_buf_pool.h"

#include <zephyr/bluetooth/l2cap.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/util.h>

namespace {

NET_BUF_POOL_DEFINE(sNetBufPool, CONFIG_ALIRO_BLE_UWB_MAX_SESSIONS, BT_L2CAP_SDU_BUF_SIZE(BT_L2CAP_SDU_TX_MTU),
		    CONFIG_BT_CONN_TX_USER_DATA_SIZE, nullptr);

} // namespace

namespace Aliro {

net_buf_pool *sL2CapServerNetBufPool{ &sNetBufPool };

} // namespace Aliro
