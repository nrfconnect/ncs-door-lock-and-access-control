#
# Copyright (c) 2025 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(door-lock-app)

file(GLOB app_sources CONFIGURE_DEPENDS src/*.cpp)

add_subdirectory(src/platform)

target_sources(app PRIVATE ${app_sources})

set(PUBLIC_API ${ZEPHYR_NCS_ALIRO_MODULE_DIR})

if(CONFIG_SOC_SERIES_NRF52X)
  set(ALIRO_LIB_PATH ${ZEPHYR_NCS_ALIRO_MODULE_DIR}/applications/doorlock/lib/aliro/bin/cortex-m4)
elseif(CONFIG_SOC_SERIES_NRF54LX OR CONFIG_SOC_SERIES_NRF53X)
  set(ALIRO_LIB_PATH ${ZEPHYR_NCS_ALIRO_MODULE_DIR}/applications/doorlock/lib/aliro/bin/cortex-m33)
endif()

add_library(aliro_stack STATIC IMPORTED GLOBAL)

if(CONFIG_ALIRO_BLE_TP)
  set_target_properties(aliro_stack PROPERTIES IMPORTED_LOCATION ${ALIRO_LIB_PATH}/libaliro_ble.a)
else(CONFIG_ALIRO_BLE_TP)
  set_target_properties(aliro_stack PROPERTIES IMPORTED_LOCATION ${ALIRO_LIB_PATH}/libaliro.a)
endif(CONFIG_ALIRO_BLE_TP)

target_link_libraries(app PRIVATE aliro_stack)
target_link_libraries(aliro_stack INTERFACE zephyr_interface)

zephyr_include_directories(
  ${PUBLIC_API}/include
  ${PUBLIC_API}/interfaces
  ${PUBLIC_API}/interfaces/crypto/backend_crypto_psa
)
