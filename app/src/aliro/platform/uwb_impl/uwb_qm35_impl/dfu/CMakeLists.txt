#
# Copyright (c) 2025 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

file(GLOB uwb_dfu_src CONFIGURE_DEPENDS *.cpp)

zephyr_library_sources(${uwb_dfu_src})
