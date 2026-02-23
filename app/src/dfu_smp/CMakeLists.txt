#
# Copyright (c) 2025 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

zephyr_include_directories(.)

zephyr_library_sources(dfu_smp_manager.cpp)

if(CONFIG_SHELL)
  zephyr_library_sources(dfu_smp_shell.cpp)
endif()

zephyr_library_link_libraries(MCUBOOT_BOOTUTIL)
