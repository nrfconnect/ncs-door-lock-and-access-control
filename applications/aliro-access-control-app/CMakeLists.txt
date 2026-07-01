#
# Copyright (c) 2025 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(door-lock-app)

include(${CMAKE_CURRENT_LIST_DIR}/../../cmake/tsrr250_nxp_overrides.cmake)
tsrr250_apply_nxp_overrides(
  ${CMAKE_CURRENT_LIST_DIR}/../../subsys/aliro/uwb/tsrr250_impl/nxp_overrides
)

if(CONFIG_DOOR_LOCK_CREDENTIAL_ISSUER_CA)
  target_sources(app PRIVATE
    ${ZEPHYR_MBEDTLS_MODULE_DIR}/library/x509.c
    ${ZEPHYR_MBEDTLS_MODULE_DIR}/library/x509_crt.c
    ${ZEPHYR_MBEDTLS_MODULE_DIR}/library/x509_oid.c
  )
  target_include_directories(app PRIVATE
    ${ZEPHYR_MBEDTLS_MODULE_DIR}/include
    ${ZEPHYR_MBEDTLS_MODULE_DIR}/library
    ${ZEPHYR_OBERON_PSA_CRYPTO_MODULE_DIR}/extras
    ${ZEPHYR_OBERON_PSA_CRYPTO_MODULE_DIR}/legacy_sub/include
    ${ZEPHYR_OBERON_PSA_CRYPTO_MODULE_DIR}/legacy_sub/src
  )
endif()

add_subdirectory(src/aliro)

file(GLOB app_sources CONFIGURE_DEPENDS src/*.cpp)
target_sources(app PRIVATE ${app_sources})
target_include_directories(app PRIVATE src)
