#
# Copyright (c) 2025 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

zephyr_library_sources(
  info.cpp
  shell.c
  shell_private.cpp
)

zephyr_library_sources_ifdef(CONFIG_BT btaddr.cpp)
zephyr_library_sources_ifdef(CONFIG_DOOR_LOCK_EXPEDITED_FAST_PHASE kpersistent.cpp)
zephyr_library_sources_ifdef(CONFIG_DOOR_LOCK_CREDENTIAL_ISSUER_CA provisioning_ci_ca.cpp)
zephyr_library_sources_ifdef(CONFIG_DOOR_LOCK_READER_CERTIFICATE reader_cert.cpp)
zephyr_library_sources_ifndef(CONFIG_CHIP
  factory_reset.cpp
  provisioning.cpp
  reader.cpp
)

zephyr_include_directories(.)
