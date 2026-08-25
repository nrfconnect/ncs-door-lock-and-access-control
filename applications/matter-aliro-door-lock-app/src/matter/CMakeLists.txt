#
# Copyright (c) 2025 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

# Enable GNU STD support.
include(${ZEPHYR_CONNECTEDHOMEIP_MODULE_DIR}/config/nrfconnect/app/enable-gnu-std.cmake)

# NORDIC SDK APP START

if(NOT CONFIG_MATTER_ZAP_FILE_PATH)
  if(CONFIG_DOOR_LOCK_BLE_UWB)
    set(CONFIG_MATTER_ZAP_FILE_PATH "${APPLICATION_CONFIG_DIR}/src/matter/zap_uwb/lock.zap")
  else()
    set(CONFIG_MATTER_ZAP_FILE_PATH "${APPLICATION_CONFIG_DIR}/src/matter/zap_nfc/lock.zap")
  endif()
endif()

# Include Data Model and ZAP utils
include(${ZEPHYR_NCS_MATTER_MODULE_DIR}/cmake/sample.cmake)

ncs_get_zap_parent_dir(ZAP_PARENT_DIR)
zephyr_include_directories(${ZAP_PARENT_DIR})

target_include_directories(app PRIVATE
  .
  ${ZAP_PARENT_DIR}
)

target_sources(app PRIVATE
  app_task.cpp
  bolt_lock_manager.cpp
  init.cpp
  time_sync_delegate.cpp
  zcl_callbacks.cpp
)

# Do not treat warnings as errors while the variable may be uninitialized for this sample data model.
# In the door-lock-server implementation, there is a warning that the C++ "optional"
# variable may be uninitialized, but actually, it is wrongly interpreted by the Zephyr toolchain.
target_compile_options(matter-data-model PRIVATE -Wno-error=maybe-uninitialized)

if(CONFIG_NCS_DOOR_LOCK_APP_EXTENSIONS_MATTER)
  include(${ZEPHYR_NCS_DOOR_LOCK_APP_EXTENSIONS_MODULE_DIR}/matter/matter_extensions.cmake)
  ncs_add_extended_data_model_sources(app matter-data-model)
  ncs_configure_extended_data_model()
else()
  ncs_configure_data_model()
endif()

# NORDIC SDK APP END
