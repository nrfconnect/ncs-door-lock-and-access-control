#
# Copyright (c) 2025 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

if(NOT TARGET aliro)
  if(CONFIG_SOC_SERIES_NRF52X)
    set(SOC_NAME "nrf52840")
  elseif(CONFIG_SOC_SERIES_NRF53X)
    set(SOC_NAME "nrf5340")
  elseif(CONFIG_SOC_NRF54L15)
    set(SOC_NAME "nrf54l15")
  elseif(CONFIG_SOC_NRF54LM20A)
    set(SOC_NAME "nrf54lm20a")
  endif()

  if(CONFIG_ALIRO_BLE_UWB)
    set(ALIRO_LIB_NAME libaliro_ble.a)
  else(CONFIG_ALIRO_BLE_UWB)
    set(ALIRO_LIB_NAME libaliro.a)
  endif(CONFIG_ALIRO_BLE_UWB)

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
