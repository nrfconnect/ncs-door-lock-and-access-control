#
# Copyright (c) 2026 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

zephyr_include_directories(include)

zephyr_library()

zephyr_library_sources(src/dfu_smp_service.cpp)
zephyr_library_sources_ifdef(CONFIG_DOOR_LOCK_DFU_SMP_SERVICE_SHELL src/shell.cpp)
zephyr_library_link_libraries(MCUBOOT_BOOTUTIL)

zephyr_library_sources_ifndef(CONFIG_CHIP
  src/advertising.cpp
  src/callbacks.cpp
)

zephyr_library_sources_ifdef(CONFIG_CHIP
  src/advertising_chip.cpp
  src/callbacks_chip.cpp
)
target_link_libraries_ifdef(CONFIG_CHIP ${ZEPHYR_CURRENT_LIBRARY} PRIVATE chip)
