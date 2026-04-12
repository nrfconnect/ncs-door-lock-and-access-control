#
# Copyright (c) 2026 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

zephyr_include_directories(include)

add_library(time_utils INTERFACE)
zephyr_append_cmake_library(time_utils)

target_sources_ifdef(CONFIG_CHIP time_utils INTERFACE src/time_utils_chip.cpp)
