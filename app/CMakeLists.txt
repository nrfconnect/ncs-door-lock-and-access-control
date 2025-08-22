#
# Copyright (c) 2025 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(door-lock-app)

file(GLOB app_sources CONFIGURE_DEPENDS src/*.cpp)

add_subdirectory(src/platform)
add_subdirectory(src/storage)

if (CONFIG_ACCESS_MANAGER_IMPLEMENTATION_DEFAULT)
  add_subdirectory(src/access_manager_impl_default)
elseif (CONFIG_ACCESS_MANAGER_IMPLEMENTATION_CUSTOM)
  add_subdirectory(src/access_manager_impl_custom)
else()
  message(FATAL_ERROR "No access manager implementation selected")
endif()

if(CONFIG_DOOR_LOCK_CLI)
  add_subdirectory(src/cli)
endif(CONFIG_DOOR_LOCK_CLI)

target_sources(app PRIVATE ${app_sources})
