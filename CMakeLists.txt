#
# Copyright (c) 2025 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

add_subdirectory(drivers)
add_subdirectory_ifdef(CONFIG_NCS_ALIRO lib/aliro)
