#
# Copyright (c) 2025 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

zephyr_include_directories(.)

if(CONFIG_ST25R500_DRV AND CONFIG_ST25R200_DRV OR NOT CONFIG_ST25R500_DRV AND NOT CONFIG_ST25R200_DRV) 
  message(FATAL_ERROR "Either CONFIG_ST25R500_DRV or CONFIG_ST25R200_DRV must be enabled, but not both")
endif()

add_subdirectory_ifdef(CONFIG_ST25R500_DRV st25r500)
add_subdirectory_ifdef(CONFIG_ST25R200_DRV st25r200)
