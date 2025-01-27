#
# Copyright (c) 2025 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

file(GLOB_RECURSE ST_PAL_SOURCES ./*.c)

zephyr_include_directories(.)

zephyr_library_sources(
    ${ST_PAL_SOURCES}
)
