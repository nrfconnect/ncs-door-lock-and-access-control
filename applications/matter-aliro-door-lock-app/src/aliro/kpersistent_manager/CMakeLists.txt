#
# Copyright (c) 2025 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

zephyr_include_directories(.)

file(GLOB src CONFIGURE_DEPENDS ./*.cpp)

zephyr_library_sources(${src})
