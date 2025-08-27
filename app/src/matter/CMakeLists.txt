#
# Copyright (c) 2025 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

# Enable GNU STD support.
include(${ZEPHYR_CONNECTEDHOMEIP_MODULE_DIR}/config/nrfconnect/app/enable-gnu-std.cmake)

# NORDIC SDK APP START

# Include all source files that are located in the Matter common directory.
include(${ZEPHYR_NRF_MODULE_DIR}/samples/matter/common/cmake/source_common.cmake)

# Include Data Model utils
include(${ZEPHYR_NRF_MODULE_DIR}/samples/matter/common/cmake/data_model.cmake)

include(${ZEPHYR_NRF_MODULE_DIR}/samples/matter/common/cmake/zap_helpers.cmake)
ncs_get_zap_parent_dir(ZAP_PARENT_DIR)

target_include_directories(app PRIVATE
  .
  ${ZAP_PARENT_DIR}
)

SET(access_srcs
  access/access_data_types.cpp
  access/access_manager_credentials.cpp
  access/access_manager_users.cpp
  access/access_manager.cpp
  access/access_storage.cpp
)

if(CONFIG_LOCK_SCHEDULES)
    LIST(APPEND access_srcs access/access_manager_schedules.cpp)
endif()

target_sources(app PRIVATE
    app_task.cpp
    bolt_lock_manager.cpp
    door_lock_delegate.cpp
    init.cpp
    zcl_callbacks.cpp
    ${access_srcs}
)

# Do not treat warnings as errors while the variable may be uninitialized for this sample data model.
# In the door-lock-server implementation, there is a warning that the C++ "optional"
# variable may be uninitialized, but actually, it is wrongly interpreted by the Zephyr toolchain.
target_compile_options(matter-data-model PRIVATE -Wno-error=maybe-uninitialized)

ncs_configure_data_model()

# NORDIC SDK APP END
