#
# Copyright (c) 2025 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

if(CONFIG_SOC_SERIES_NRF53)
  set(SOC_NAME "nrf5340")
elseif(CONFIG_SOC_NRF54LM20A)
  set(SOC_NAME "nrf54lm20a")
else()
  message(FATAL_ERROR "Unsupported SOC")
endif()

zephyr_include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/include/qosal
  ${CMAKE_CURRENT_SOURCE_DIR}/include/qosal/zephyr
  ${CMAKE_CURRENT_SOURCE_DIR}/include/qmrom
)

zephyr_library_import(aliro_uwb_adapter
  ${CMAKE_CURRENT_SOURCE_DIR}/bin/${SOC_NAME}/libaliro_uwb_adapter.a
)

zephyr_library_import(cherry_core_impl
  ${CMAKE_CURRENT_SOURCE_DIR}/bin/${SOC_NAME}/libcherry_core_impl.a
)

zephyr_library_import(cherry_uci_core_client
  ${CMAKE_CURRENT_SOURCE_DIR}/bin/${SOC_NAME}/libcherry_uci_core_client.a
)

zephyr_library_import(cherry_ccc_impl
  ${CMAKE_CURRENT_SOURCE_DIR}/bin/${SOC_NAME}/libcherry_ccc_impl.a
)

zephyr_library_import(cherry_uci_ccc_client
  ${CMAKE_CURRENT_SOURCE_DIR}/bin/${SOC_NAME}/libcherry_uci_ccc_client.a
)

zephyr_library_import(cherry_radar_impl
  ${CMAKE_CURRENT_SOURCE_DIR}/bin/${SOC_NAME}/libcherry_radar_impl.a
)

zephyr_library_import(cherry_uci_radar_client
  ${CMAKE_CURRENT_SOURCE_DIR}/bin/${SOC_NAME}/libcherry_uci_radar_client.a
)

zephyr_library_import(hsspi
  ${CMAKE_CURRENT_SOURCE_DIR}/bin/${SOC_NAME}/libhsspi.a
)

zephyr_library_import(qmutils
  ${CMAKE_CURRENT_SOURCE_DIR}/bin/${SOC_NAME}/libqmutils.a
)

zephyr_library_import(qosal
  ${CMAKE_CURRENT_SOURCE_DIR}/bin/${SOC_NAME}/libqosal.a
)

zephyr_library_import(uci
  ${CMAKE_CURRENT_SOURCE_DIR}/bin/${SOC_NAME}/libuci.a
)

zephyr_library_import(libqmrom
  ${CMAKE_CURRENT_SOURCE_DIR}/bin/${SOC_NAME}/liblibqmrom.a
)
