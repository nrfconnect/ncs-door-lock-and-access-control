#
# Copyright (c) 2025 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

file(GLOB app_sources CONFIGURE_DEPENDS *.cpp)

add_subdirectory(platform)
add_subdirectory(storage)

if (CONFIG_ACCESS_MANAGER_IMPLEMENTATION_DEFAULT)
  add_subdirectory(access_manager_impl_default)
elseif (CONFIG_ACCESS_MANAGER_IMPLEMENTATION_CUSTOM)
  add_subdirectory(access_manager_impl_custom)
else()
  message(FATAL_ERROR "No access manager implementation selected")
endif()

if(CONFIG_DOOR_LOCK_CLI)
  add_subdirectory(cli)
endif(CONFIG_DOOR_LOCK_CLI)

target_sources(app PRIVATE ${app_sources})
