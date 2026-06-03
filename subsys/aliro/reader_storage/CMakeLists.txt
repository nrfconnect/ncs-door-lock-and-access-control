#
# Copyright (c) 2026 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

zephyr_include_directories(include)

zephyr_library()

zephyr_library_include_directories(src)

zephyr_library_sources(
  src/core.cpp
  src/reader.cpp
)

zephyr_library_sources_ifdef(CONFIG_DOOR_LOCK_ALIRO_READER_STORAGE_GROUP_RESOLVING_KEY_SUPPORT src/group.cpp)
zephyr_library_sources_ifdef(CONFIG_DOOR_LOCK_ALIRO_READER_STORAGE_CERTIFICATE_SUPPORT src/cert.cpp)

add_subdirectory_ifdef(CONFIG_DOOR_LOCK_ALIRO_READER_STORAGE_SHELL src/shell)
