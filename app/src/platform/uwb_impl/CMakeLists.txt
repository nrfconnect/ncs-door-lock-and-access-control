#
# Copyright (c) 2025 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

zephyr_include_directories(.)

if(CONFIG_QM35_UWB_ALIRO_ZEPHYR OR CONFIG_QM35_UWB_LIB)
  add_subdirectory(uwb_qm35_impl)
else()
  add_subdirectory(uwb_dummy_impl)
endif()
