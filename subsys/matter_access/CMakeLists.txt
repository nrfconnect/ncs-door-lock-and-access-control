#
# Copyright (c) 2026 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

zephyr_include_directories(include)

zephyr_library_get_current_dir_lib_name(${ZEPHYR_BASE} lib_name)
add_library(${lib_name} INTERFACE)
zephyr_append_cmake_library(${lib_name})

target_sources(${lib_name} INTERFACE
  src/access_data_types.cpp
  src/access_manager.cpp
  src/access_manager_credentials.cpp
  src/access_manager_users.cpp
)

target_sources_ifdef(CONFIG_DOOR_LOCK_MATTER_ACCESS_STORAGE_PROTECTED_STORAGE ${lib_name} INTERFACE src/access_storage_psa_ps.cpp)
target_sources_ifdef(CONFIG_DOOR_LOCK_MATTER_ACCESS_SCHEDULES ${lib_name} INTERFACE src/access_manager_schedules.cpp)
target_sources_ifdef(CONFIG_DOOR_LOCK_MATTER_ACCESS_PRINT_STORAGE_STATUS ${lib_name} INTERFACE src/access_storage_print.cpp)

if(CONFIG_DOOR_LOCK_MATTER_ACCESS_STORAGE_PERSISTENT_STORAGE)
  target_sources(${lib_name} INTERFACE src/access_storage.cpp)
endif()

# The Matter headers used here include the ZAP output of the application
# data model, which is generated into the build directory at build time.
if(CONFIG_MATTER_ZAP_GENERATION_BUILD_TIME)
  target_include_directories(${lib_name} INTERFACE
    $<TARGET_PROPERTY:matter-data-model,INCLUDE_DIRECTORIES>
  )
  add_dependencies(${lib_name} matter-data-model-zapgen matter-data-model-codegen)
endif()
