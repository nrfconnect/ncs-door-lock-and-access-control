#
# Copyright (c) 2026 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

zephyr_include_directories(include)

zephyr_library_get_current_dir_lib_name(${ZEPHYR_BASE} lib_name)
add_library(${lib_name} INTERFACE)
zephyr_append_cmake_library(${lib_name})

target_sources_ifdef(CONFIG_CHIP ${lib_name} INTERFACE src/door_lock_delegate.cpp)
