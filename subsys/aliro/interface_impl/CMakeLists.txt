#
# Copyright (c) 2026 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

add_subdirectory_ifdef(CONFIG_DOOR_LOCK_ALIRO_INTERFACE_IMPL_OS os)
add_subdirectory_ifdef(CONFIG_DOOR_LOCK_ALIRO_INTERFACE_IMPL_LOG log)
add_subdirectory_ifdef(CONFIG_DOOR_LOCK_ALIRO_INTERFACE_IMPL_UWB uwb)
add_subdirectory_ifdef(CONFIG_DOOR_LOCK_ALIRO_INTERFACE_IMPL_READER reader)
add_subdirectory_ifdef(CONFIG_DOOR_LOCK_ALIRO_INTERFACE_IMPL_CRYPTO crypto)
