#
# Copyright (c) 2025 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

zephyr_include_directories(.)

zephyr_library_sources(timer.cpp)

add_subdirectory(nfc)

if(CONFIG_BT)
  add_subdirectory(ble)
endif()

add_subdirectory_ifdef(CONFIG_DOOR_LOCK_BLE_UWB uwb_impl)
