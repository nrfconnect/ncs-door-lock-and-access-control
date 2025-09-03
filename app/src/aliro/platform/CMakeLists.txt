#
# Copyright (c) 2025 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

zephyr_include_directories(.)

add_subdirectory(nfc_transport_impl)
add_subdirectory(logger)

add_subdirectory_ifdef(CONFIG_ALIRO_BLE_UWB ble)
add_subdirectory_ifdef(CONFIG_ALIRO_BLE_UWB uwb_impl)
add_subdirectory_ifdef(CONFIG_ACCESS_DECISION_INDICATOR access_decision_indicator)
