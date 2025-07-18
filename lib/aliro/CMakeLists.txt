#
# Copyright (c) 2025 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

if(CONFIG_SOC_SERIES_NRF52X)
  set(ALIRO_LIB_PATH ${CMAKE_CURRENT_SOURCE_DIR}/bin/cortex-m4)
elseif(CONFIG_SOC_SERIES_NRF54LX OR CONFIG_SOC_SERIES_NRF53X)
  set(ALIRO_LIB_PATH ${CMAKE_CURRENT_SOURCE_DIR}/bin/cortex-m33)
endif()

if(CONFIG_ALIRO_BLE_TP)
  set(ALIRO_LIB_NAME libaliro_ble.a)
else(CONFIG_ALIRO_BLE_TP)
  set(ALIRO_LIB_NAME libaliro.a)
endif(CONFIG_ALIRO_BLE_TP)

zephyr_library_import(aliro_stack ${ALIRO_LIB_PATH}/${ALIRO_LIB_NAME})

if(NOT DEFINED PUBLIC_API_DIR)
  set(PUBLIC_API_DIR ${CMAKE_CURRENT_SOURCE_DIR})
endif()

zephyr_include_directories(
  ${PUBLIC_API_DIR}/include
  ${PUBLIC_API_DIR}/interfaces
  ${PUBLIC_API_DIR}/interfaces/crypto/backend_crypto_psa
)
