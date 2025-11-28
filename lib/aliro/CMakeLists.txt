#
# Copyright (c) 2025 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

if(NOT TARGET aliro)
  if(CONFIG_DOOR_LOCK_BLE_UWB)
    set(ALIRO_LIB_NAME libaliro_ble.a)
  else(CONFIG_DOOR_LOCK_BLE_UWB)
    set(ALIRO_LIB_NAME libaliro.a)
  endif(CONFIG_DOOR_LOCK_BLE_UWB)

  zephyr_library_import(aliro ${CMAKE_CURRENT_SOURCE_DIR}/bin/${SOC_NAME}/${ALIRO_LIB_NAME})
endif()

if(NOT DEFINED PUBLIC_API_DIR)
  set(PUBLIC_API_DIR ${CMAKE_CURRENT_SOURCE_DIR})
endif()

zephyr_include_directories(
  ${PUBLIC_API_DIR}/include
  ${PUBLIC_API_DIR}/interfaces
  ${PUBLIC_API_DIR}/interfaces/crypto/backend_crypto_psa
)
