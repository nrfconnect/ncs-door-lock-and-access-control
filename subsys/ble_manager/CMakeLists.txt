#
# Copyright (c) 2026 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

zephyr_library()

zephyr_library_sources(src/ble_manager.cpp)
zephyr_library_sources_ifdef(CONFIG_DOOR_LOCK_BLE_MANAGER_INIT src/ble_manager_init.cpp)
zephyr_library_sources_ifdef(CONFIG_DOOR_LOCK_BLE_MANAGER_AUTH src/ble_manager_auth.cpp)
