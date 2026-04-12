#
# Copyright (c) 2026 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

add_subdirectory_ifdef(CONFIG_DOOR_LOCK_ALIRO_ACCESS_DOCUMENT_PARSER access_document)
add_subdirectory_ifdef(CONFIG_DOOR_LOCK_ALIRO_WORKQUEUE aliro_workqueue)
add_subdirectory_ifdef(CONFIG_DOOR_LOCK_ALIRO_CRYPTO_UTILS crypto_utils)
add_subdirectory_ifdef(CONFIG_DOOR_LOCK_ALIRO_TIME_UTILS time_utils)
