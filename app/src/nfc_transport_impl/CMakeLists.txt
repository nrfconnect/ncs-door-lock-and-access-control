#
# Copyright (c) 2025 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

zephyr_include_directories(.)

FILE(GLOB impl_src *.cpp)

zephyr_library_sources(
	${impl_src}
)
