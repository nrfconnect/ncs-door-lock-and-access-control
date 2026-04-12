#
# Copyright (c) 2026 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

add_subdirectory(aliro)

add_subdirectory_ifdef(CONFIG_DOOR_LOCK_EXTERNAL_NVS external_nvs)
add_subdirectory_ifdef(CONFIG_DOOR_LOCK_MATTER_ACCESS matter_access)
add_subdirectory_ifdef(CONFIG_DOOR_LOCK_SETTINGS_UTILS settings_utils)
