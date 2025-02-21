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
add_subdirectory(src/storage)
add_subdirectory(src/cli)

target_sources(app PRIVATE
    ${app_sources}
)

if (CONFIG_NCS_ALIRO_LIB)

    set(ALIRO_LIB ${ZEPHYR_NCS_DOOR_LOCK_APP_MODULE_DIR}/lib/aliro)

    zephyr_link_libraries(${ALIRO_LIB}/bin/libaliro.a)
    zephyr_include_directories(${ALIRO_LIB}/include)
    zephyr_include_directories(${ALIRO_LIB}/interfaces)
    zephyr_include_directories(${ALIRO_LIB}/interfaces/crypto/backend_crypto_psa)

endif()
