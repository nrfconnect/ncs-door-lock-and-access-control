#
# Copyright (c) 2025 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

zephyr_include_directories(.)

add_subdirectory(crypto)
add_subdirectory(logger)
add_subdirectory(nfc)

if(CONFIG_BT)
  add_subdirectory(ble)
endif()

add_subdirectory_ifdef(CONFIG_DOOR_LOCK_BLE_UWB uwb_impl)
add_subdirectory_ifdef(CONFIG_ACCESS_DECISION_INDICATOR access_decision_indicator)
