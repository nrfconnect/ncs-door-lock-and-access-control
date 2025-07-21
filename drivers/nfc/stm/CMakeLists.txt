#
# Copyright (c) 2025 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

add_subdirectory(PAL)
add_subdirectory(nfc_configs)

set(ST_RFAL_DIR RFAL)
set(ST_RFAL_SRC_DIR ${ST_RFAL_DIR}/source)
set(ST_RFAL_INC_DIR ${ST_RFAL_DIR}/include)

set(INCLUDE_DIR include)

if(CONFIG_ST25R3911_DRV)
  zephyr_library_compile_definitions(-DST25R3911)
  set(ST_DRIVER_INC ${ST_RFAL_SRC_DIR}/st25r3911)
elseif(CONFIG_ST25R3916B_DRV OR CONFIG_ST25R3916_DRV)
  zephyr_library_compile_definitions_ifdef(CONFIG_ST25R3916_DRV -DST25R3916)
  zephyr_library_compile_definitions_ifdef(CONFIG_ST25R3916B_DRV -DST25R3916B)
  set(ST_DRIVER_INC ${ST_RFAL_SRC_DIR}/st25r3916)
elseif(CONFIG_ST25R200_DRV)
  zephyr_library_compile_definitions(-DST25R200)
  set(ST_DRIVER_INC ${ST_RFAL_SRC_DIR}/st25r200)
else()
  message(FATAL_ERROR "NFC driver not defined")
endif()

file(GLOB ST_RFAL_SOURCES CONFIGURE_DEPENDS ${ST_RFAL_SRC_DIR}/*.c)
file(GLOB_RECURSE ST_DRIVER_SOURCES CONFIGURE_DEPENDS ${ST_DRIVER_INC}/*.c)

zephyr_include_directories(
  ${ST_DRIVER_INC}
  ${ST_RFAL_INC_DIR}
  ${ST_RFAL_SRC_DIR}
  ${INCLUDE_DIR}
)

zephyr_library_sources(
  ${ST_DRIVER_SOURCES}
  ${ST_RFAL_SOURCES}
)
