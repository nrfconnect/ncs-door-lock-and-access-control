#
# Copyright (c) 2026 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

zephyr_include_directories(include)

zephyr_library()

zephyr_library_sources(src/nus_service.cpp)

zephyr_library_sources_ifndef(CONFIG_CHIP src/advertising.cpp)

zephyr_library_sources_ifdef(CONFIG_CHIP src/advertising_chip.cpp)
target_link_libraries_ifdef(CONFIG_CHIP ${ZEPHYR_CURRENT_LIBRARY} PRIVATE chip)
