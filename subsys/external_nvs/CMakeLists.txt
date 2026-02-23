#
# Copyright (c) 2026 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

zephyr_include_directories(include)
zephyr_sources(
  src/aead.cpp
  src/external_nvs.cpp
  src/key.cpp
  src/nonce.cpp
  src/storage.cpp
)

zephyr_sources_ifdef(CONFIG_DOOR_LOCK_EXTERNAL_NVS_ROLLBACK_PROTECTION src/counter.cpp)
