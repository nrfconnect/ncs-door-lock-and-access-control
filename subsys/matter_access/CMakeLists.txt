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
  set(MATTER_COMMONS_SRC_DIR ${ZEPHYR_NRF_MODULE_DIR}/samples/matter/common/src)
  zephyr_include_directories(${MATTER_COMMONS_SRC_DIR})
  target_sources(${lib_name} INTERFACE src/access_storage.cpp)
endif()
