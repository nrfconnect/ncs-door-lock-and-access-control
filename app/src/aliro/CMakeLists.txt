#
# Copyright (c) 2025 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

target_sources(app PRIVATE
	init.cpp
)

if(NOT CONFIG_CHIP)
	set(ALIRO_STANDALONE TRUE)
endif()

target_sources_ifdef(ALIRO_STANDALONE app PRIVATE
	aliro_state_control.cpp
)

add_subdirectory(access_manager)
add_subdirectory(aliro_work)
add_subdirectory(interface_impl)
add_subdirectory(lock_sim)
add_subdirectory(platform)
add_subdirectory(storage)

add_subdirectory_ifdef(CONFIG_DOOR_LOCK_STEP_UP_PHASE access_document)
add_subdirectory_ifdef(CONFIG_DOOR_LOCK_EXPEDITED_FAST_PHASE kpersistent_manager)
add_subdirectory_ifdef(CONFIG_DOOR_LOCK_CLI cli)
