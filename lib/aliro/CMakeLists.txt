#
# Copyright (c) 2025 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

if(NOT TARGET aliro)
  if(CONFIG_NCS_ALIRO_BLE_UWB)
    set(ALIRO_LIB_NAME libaliro_ble.a)
  else(CONFIG_NCS_ALIRO_BLE_UWB)
    set(ALIRO_LIB_NAME libaliro.a)
  endif(CONFIG_NCS_ALIRO_BLE_UWB)

  if(CONFIG_DOOR_LOCK_RELEASE)
    set(BUILD_TYPE "release")
  else()
    set(BUILD_TYPE "debug")
  endif()

  if(CONFIG_CPU_CORTEX_M33)
    set(CPU_NAME "cortex-m33")
  elseif(CONFIG_CPU_CORTEX_M4)
    set(CPU_NAME "cortex-m4")
  else()
    message(FATAL_ERROR "Unsupported CPU")
  endif()

  set(ALIRO_LIB_PATH ${CMAKE_CURRENT_SOURCE_DIR}/bin/${BUILD_TYPE}/${CPU_NAME}/${ALIRO_LIB_NAME})

  if(NOT EXISTS ${ALIRO_LIB_PATH})
    message(FATAL_ERROR "Aliro library not found. Expected at ${ALIRO_LIB_PATH}")
  endif()

  zephyr_library_import(aliro ${ALIRO_LIB_PATH})
endif()

if(NOT DEFINED PUBLIC_API_DIR)
  set(PUBLIC_API_DIR ${CMAKE_CURRENT_SOURCE_DIR})
endif()

zephyr_include_directories(
  ${PUBLIC_API_DIR}/include
)
