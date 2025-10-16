#
# Copyright (c) 2025 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

file(GLOB app_sources CONFIGURE_DEPENDS *.cpp)

add_subdirectory(access_document)
add_subdirectory(platform)
add_subdirectory(storage)

add_subdirectory_ifdef(CONFIG_ACCESS_MANAGER_IMPLEMENTATION_DEFAULT access_manager_impl_default)
add_subdirectory_ifdef(CONFIG_ACCESS_MANAGER_IMPLEMENTATION_CUSTOM access_manager_impl_custom)
add_subdirectory_ifdef(CONFIG_DOOR_LOCK_EXPEDITED_FAST_PHASE kpersistent_manager)
add_subdirectory_ifdef(CONFIG_DOOR_LOCK_CLI cli)

target_sources(app PRIVATE ${app_sources})
