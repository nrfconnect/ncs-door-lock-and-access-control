#
# Copyright (c) 2021 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(matter-lock)

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

# Without the OpenThread shell and logging, nothing references the OpenThread API objects before
# libCHIP.a is scanned, so symbols used only by Matter's Thread platform stay unresolved. Link the
# OpenThread library again after Matter, so that the linker can resolve them.
if(CONFIG_OPENTHREAD_SOURCES)
  if(CONFIG_OPENTHREAD_FTD)
    set(ot_core_lib openthread-ftd)
  else()
    set(ot_core_lib openthread-mtd)
  endif()
  target_link_libraries(chip INTERFACE
    -Wl,--start-group
    $<TARGET_FILE:${ot_core_lib}>
    -Wl,--end-group
  )
endif()

ncs_configure_data_model()

# NORDIC SDK APP END
