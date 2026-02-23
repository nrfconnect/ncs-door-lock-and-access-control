#
# Copyright (c) 2025 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

target_sources(app PRIVATE
  reader_cache.cpp
  storage.cpp
)
target_sources_ifdef(CONFIG_DOOR_LOCK_STEP_UP_PHASE app PRIVATE validity_iterations.cpp)

if(CONFIG_DOOR_LOCK_STEP_UP_PHASE AND CONFIG_DOOR_LOCK_STORAGE_MAX_STORED_ACCESS_DOCUMENTS GREATER 0)
  target_sources(app PRIVATE access_document.cpp)
endif()

target_include_directories(app PRIVATE .)
