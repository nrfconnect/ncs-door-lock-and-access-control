#
# Copyright (c) 2025 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

zephyr_include_directories(.)

add_subdirectory_ifdef(CONFIG_DOOR_LOCK_BLE_UWB gatt_server)
add_subdirectory_ifdef(CONFIG_DOOR_LOCK_BLE_UWB l2cap_server)

# BLE Manager (always included for BT builds)
zephyr_library_sources(ble_manager.cpp)

# BLE Advertising Arbiter - select implementation based on Matter support
if(CONFIG_CHIP)
    zephyr_library_sources(ble_advertising_arbiter_chip.cpp)
else()
    zephyr_library_sources(ble_advertising_arbiter.cpp)
endif()
