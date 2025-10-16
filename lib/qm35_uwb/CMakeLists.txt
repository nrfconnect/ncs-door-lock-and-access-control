#
# Copyright (c) 2025 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

zephyr_include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/include/qosal
  ${CMAKE_CURRENT_SOURCE_DIR}/include/qosal/zephyr
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
