#
# Copyright (c) 2025 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

file(GLOB app_sources CONFIGURE_DEPENDS *.cpp)

add_subdirectory(platform)
add_subdirectory(storage)

add_subdirectory_ifdef(CONFIG_DOOR_LOCK_STEP_UP_PHASE access_document)
add_subdirectory_ifdef(CONFIG_DOOR_LOCK_ACCESS_MANAGER_IMPLEMENTATION_DEFAULT access_manager_impl_default)
add_subdirectory_ifdef(CONFIG_DOOR_LOCK_ACCESS_MANAGER_IMPLEMENTATION_CUSTOM access_manager_impl_custom)
add_subdirectory_ifdef(CONFIG_DOOR_LOCK_EXPEDITED_FAST_PHASE kpersistent_manager)
add_subdirectory_ifdef(CONFIG_DOOR_LOCK_CLI cli)
add_subdirectory_ifndef(CONFIG_CHIP lock_sim)

target_sources(app PRIVATE ${app_sources})
