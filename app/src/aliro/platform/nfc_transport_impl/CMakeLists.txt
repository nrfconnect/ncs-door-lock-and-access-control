#
# Copyright (c) 2025 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

zephyr_include_directories(.)

# Core RFAL transport implementation
zephyr_library_sources(nfc_transport_rfal.cpp)

zephyr_library_sources_ifdef(CONFIG_DOOR_LOCK_NFC_PROP nfc_transport_rfal_prop.cpp)
