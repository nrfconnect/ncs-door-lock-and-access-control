#
# Copyright (c) 2025 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

target_sources(app PRIVATE
	aliro_state_control.cpp
	init.cpp
)

add_subdirectory(access_manager)
add_subdirectory(interface_impl)
add_subdirectory(platform)
add_subdirectory(storage)

add_subdirectory_ifdef(CONFIG_DOOR_LOCK_EXPEDITED_FAST_PHASE kpersistent_manager)
add_subdirectory_ifdef(CONFIG_DOOR_LOCK_CLI cli)
