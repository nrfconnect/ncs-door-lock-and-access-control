#
# Copyright (c) 2025 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

zephyr_include_directories(.)

add_subdirectory_ifdef(CONFIG_DOOR_LOCK_BLE_UWB gatt_server)
add_subdirectory_ifdef(CONFIG_DOOR_LOCK_BLE_UWB l2cap_server)
add_subdirectory_ifdef(CONFIG_DOOR_LOCK_BLE_UWB ble_interface_impl)

file(GLOB ble_src CONFIGURE_DEPENDS *.cpp)

zephyr_library_sources(${ble_src})
