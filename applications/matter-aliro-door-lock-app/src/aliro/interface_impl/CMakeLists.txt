#
# Copyright (c) 2026 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

target_sources(app PRIVATE
  access_document.cpp
  access.cpp
  ci_cert.cpp
  crypto.cpp
  os_mutex.cpp
  os_timer.cpp
  os.cpp
  reader.cpp
  session.cpp
)

target_sources_ifdef(CONFIG_NCS_ALIRO_BLE_UWB app PRIVATE
  ble.cpp
  uwb.cpp
)

if(CONFIG_NCS_ALIRO_LOG_LEVEL_VALUE GREATER 0)
  target_sources(app PRIVATE log.cpp)
endif()
