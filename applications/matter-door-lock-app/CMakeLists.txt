#
# Copyright (c) 2021 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(matter-lock)

# Enable GNU STD support.
include(${ZEPHYR_CONNECTEDHOMEIP_MODULE_DIR}/config/nrfconnect/app/enable-gnu-std.cmake)

# NORDIC SDK APP START

# Include Data Model and ZAP utils
include(${ZEPHYR_NCS_MATTER_MODULE_DIR}/cmake/sample.cmake)

ncs_get_zap_parent_dir(ZAP_PARENT_DIR)
zephyr_include_directories(${ZAP_PARENT_DIR})

target_include_directories(app PRIVATE
  src
)

target_sources(app PRIVATE
  src/app_task.cpp
  src/bolt_lock_manager.cpp
  src/main.cpp
  src/zcl_callbacks.cpp
)

# Do not treat warnings as errors while the variable may be uninitialized for this sample data model.
# In the door-lock-server implementation, there is a warning that the C++ "optional"
# variable may be uninitialized, but actually, it is wrongly interpreted by the Zephyr toolchain.
target_compile_options(matter-data-model PRIVATE -Wno-error=maybe-uninitialized)

ncs_configure_data_model()

# NORDIC SDK APP END
