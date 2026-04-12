#
# Copyright (c) 2026 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

zephyr_include_directories(include)

add_library(matter_access INTERFACE)
zephyr_append_cmake_library(matter_access)

target_sources(matter_access INTERFACE
  src/access_data_types.cpp
  src/access_manager.cpp
  src/access_manager_credentials.cpp
  src/access_manager_users.cpp
  src/access_storage_psa_ps.cpp
)

target_sources_ifdef(CONFIG_DOOR_LOCK_MATTER_ACCESS_SCHEDULES matter_access INTERFACE src/access_manager_schedules.cpp)
target_sources_ifdef(CONFIG_DOOR_LOCK_MATTER_ACCESS_PRINT_STORAGE_STATUS matter_access INTERFACE src/access_storage_print.cpp)
