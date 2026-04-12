#
# Copyright (c) 2026 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

zephyr_include_directories(include)

zephyr_library_named(crypto_utils)

zephyr_library_sources(
  src/crypto_utils.cpp
)
