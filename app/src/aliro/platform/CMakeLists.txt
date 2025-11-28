#
# Copyright (c) 2025 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

zephyr_include_directories(.)

add_subdirectory(nfc_transport_impl)
add_subdirectory(logger)

if(CONFIG_DOOR_LOCK_BLE_UWB OR CONFIG_DOOR_LOCK_DFU_BLE_SMP OR CONFIG_DOOR_LOCK_BLE_NUS)
  add_subdirectory(ble)
endif()

add_subdirectory_ifdef(CONFIG_DOOR_LOCK_BLE_UWB uwb_impl)
add_subdirectory_ifdef(CONFIG_ACCESS_DECISION_INDICATOR access_decision_indicator)
