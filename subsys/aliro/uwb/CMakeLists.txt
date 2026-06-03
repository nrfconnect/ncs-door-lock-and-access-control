#
# Copyright (c) 2025 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

zephyr_include_directories(.)

zephyr_library_sources(uwb.cpp)

if(CONFIG_QM35_UWB_ALIRO_ZEPHYR OR CONFIG_QM35_UWB_LIB)
  add_subdirectory(qm35_impl)
else()
  add_subdirectory(custom_impl)
endif()
