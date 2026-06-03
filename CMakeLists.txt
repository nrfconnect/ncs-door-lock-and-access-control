#
# Copyright (c) 2025 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)

zephyr_include_directories(include)

add_subdirectory(drivers)
add_subdirectory(lib)
add_subdirectory(subsys)
