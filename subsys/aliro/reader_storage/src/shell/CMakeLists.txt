#
# Copyright (c) 2026 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

zephyr_library_sources(
  shell.c
)

zephyr_library_sources_ifdef(CONFIG_DOOR_LOCK_ALIRO_READER_STORAGE_SHELL_CORE core.cpp)
zephyr_library_sources_ifdef(CONFIG_DOOR_LOCK_ALIRO_READER_STORAGE_SHELL_CERT cert.cpp)
zephyr_library_sources_ifdef(CONFIG_DOOR_LOCK_ALIRO_READER_STORAGE_SHELL_GROUP group.cpp)
