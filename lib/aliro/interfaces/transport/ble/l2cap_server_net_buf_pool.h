/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

#pragma once

#include <zephyr/net_buf.h>

namespace Aliro {

/**
 * @brief L2CAP server network buffer pool.
 *
 * Provides network buffer pool for Aliro BLE L2CAP server.
 *
 * Due to Zephyr API constraints, network buffer pool has to be statically
 * allocated by the application. Pool size must be configured based on the
 * maximum number of supported BLE sessions.
 */
extern net_buf_pool *sL2CapServerNetBufPool;

} // namespace Aliro
