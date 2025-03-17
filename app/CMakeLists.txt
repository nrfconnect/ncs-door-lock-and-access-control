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

set(ALIRO_LIB ${ZEPHYR_NCS_DOOR_LOCK_APP_MODULE_DIR}/lib/aliro/bin)
set(PUBLIC_API ${ZEPHYR_NCS_DOOR_LOCK_APP_MODULE_DIR}/lib/aliro)

zephyr_link_libraries(${ALIRO_LIB}/libaliro.a)

zephyr_include_directories(${PUBLIC_API}/include)
zephyr_include_directories(${PUBLIC_API}/interfaces)
zephyr_include_directories(${PUBLIC_API}/interfaces/crypto/backend_crypto_psa)
