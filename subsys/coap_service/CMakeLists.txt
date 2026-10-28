#
# Copyright (c) 2026 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

zephyr_include_directories(include ${CMAKE_CURRENT_SOURCE_DIR}/../../lib/aliro/include)

zephyr_library()

zephyr_library_sources(src/coap_service.cpp)

zephyr_linker_sources(DATA_SECTIONS sections-ram.ld)
zephyr_iterable_section(
  NAME coap_resource_door_lock_service
  GROUP DATA_REGION ${XIP_ALIGN_WITH_INPUT})
