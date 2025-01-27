#
# Copyright (c) 2025 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

add_subdirectory(PAL)

set(ST_RFAL_DIR RFAL)
set(ST_RFAL_SRC_DIR ${ST_RFAL_DIR}/source)
set(ST_RFAL_INC_DIR ${ST_RFAL_DIR}/include)

set(INCLUDE_DIR include)

if(CONFIG_ST25R3911_DRV)
    zephyr_library_compile_definitions(-DST25R3911) 
    set(ST_DRIVER_INC ${ST_RFAL_SRC_DIR}/st25r3911)
else(CONFIG_ST25R3911_DRV)
    message(FATAL_ERROR "NFC driver not defined")
endif(CONFIG_ST25R3911_DRV)

file(GLOB ST_RFAL_SOURCES ${ST_RFAL_SRC_DIR}/*.c)
file(GLOB_RECURSE ST_DRIVER_SOURCES ${ST_DRIVER_INC}/*.c)

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
