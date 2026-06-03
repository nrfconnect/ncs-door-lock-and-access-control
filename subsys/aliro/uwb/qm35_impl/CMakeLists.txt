#
# Copyright (c) 2025 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

zephyr_include_directories(.)

file(GLOB uwb_src CONFIGURE_DEPENDS *.cpp)

zephyr_library_sources(${uwb_src})

zephyr_library_compile_definitions(HAVE_HSSPI_SPI)

if(CONFIG_DOOR_LOCK_ALIRO_UWB_QM35_DISAMBIGUATION AND NOT CONFIG_QM35_UWB_LIB)
  zephyr_library_link_libraries(cherry_radar)
endif()

add_subdirectory_ifdef(CONFIG_SHELL cli)
add_subdirectory_ifdef(CONFIG_DOOR_LOCK_ALIRO_UWB_QM35_DFU dfu)
add_subdirectory_ifdef(CONFIG_DOOR_LOCK_ALIRO_UWB_QM35_DISAMBIGUATION disambiguation)
