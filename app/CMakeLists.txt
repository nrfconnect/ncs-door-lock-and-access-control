#
# Copyright (c) 2025 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(door-lock-app)

FILE(GLOB app_sources src/*.cpp)

add_subdirectory(src/nfc_transport_impl)

target_sources(app PRIVATE
    ${app_sources}
)

set(ALIRO_LIB_DIR ${ZEPHYR_NCS_DOOR_LOCK_APP_MODULE_DIR}/lib/aliro)

if(CONFIG_SOC_SERIES_NRF52X)
    set(ALIRO_LIB_BIN_PATH ${ALIRO_LIB_DIR}/bin/cortex-m4)
elseif(CONFIG_SOC_SERIES_NRF54LX OR CONFIG_SOC_SERIES_NRF53X)
    set(ALIRO_LIB_BIN_PATH ${ALIRO_LIB_DIR}/bin/cortex-m33)
endif()

add_library(aliro_stack STATIC IMPORTED GLOBAL)

set_target_properties(aliro_stack PROPERTIES IMPORTED_LOCATION ${ALIRO_LIB_BIN_PATH}/libaliro.a)

target_link_libraries(app PRIVATE aliro_stack)
target_link_libraries(aliro_stack INTERFACE zephyr_interface)

zephyr_include_directories(${ALIRO_LIB_DIR}/include)
zephyr_include_directories(${ALIRO_LIB_DIR}/interfaces)
zephyr_include_directories(${ALIRO_LIB_DIR}/interfaces/crypto/backend_crypto_psa)
