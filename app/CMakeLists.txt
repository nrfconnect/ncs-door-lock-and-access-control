#
# Copyright (c) 2025 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(door-lock-app)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/../cmake)

add_subdirectory(src/aliro)
add_subdirectory_ifdef(CONFIG_CHIP src/matter)
add_subdirectory_ifdef(CONFIG_DOOR_LOCK_DFU_BLE_SMP src/dfu_smp)
add_subdirectory_ifdef(CONFIG_DOOR_LOCK_BLE_NUS src/bt_nus)

file(GLOB app_sources CONFIGURE_DEPENDS src/*.cpp)
target_sources(app PRIVATE ${app_sources})
target_sources_ifdef(CONFIG_USE_SEGGER_RTT app PRIVATE src/fatal_rtt.c)
target_include_directories(app PRIVATE src)
