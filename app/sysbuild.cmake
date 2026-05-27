#
# Copyright (c) 2025 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

if("matter" IN_LIST SNIPPET)

  # Set the matter-application-core snippet for the application core.
  if(NOT "matter-application-core" IN_LIST ${DEFAULT_IMAGE}_SNIPPET)
    set(${DEFAULT_IMAGE}_SNIPPET ${${DEFAULT_IMAGE}_SNIPPET} matter-application-core CACHE STRING "" FORCE)
  endif()

  # Set the matter-network-core snippet for devices that support network core.
  if(SB_CONFIG_NRF_DEFAULT_IPC_RADIO)
    if(NOT "matter-network-core" IN_LIST ipc_radio_SNIPPET)
      set(ipc_radio_SNIPPET ${ipc_radio_SNIPPET} matter-network-core CACHE STRING "" FORCE)
    endif()
  endif()

endif()

if("uwb_qm35_dfu" IN_LIST SNIPPET)
  set(PM_STATIC_YML_FILE ${CMAKE_CURRENT_LIST_DIR}/pm_static_${NORMALIZED_BOARD_TARGET}_uwb_dfu.yml CACHE INTERNAL "")

  # Set the uwb_qm35_dfu_app for app image
  if(NOT "uwb_qm35_dfu_app" IN_LIST ${DEFAULT_IMAGE}_SNIPPET)
    set(${DEFAULT_IMAGE}_SNIPPET ${${DEFAULT_IMAGE}_SNIPPET} uwb_qm35_dfu_app CACHE STRING "" FORCE)
  endif()

  if(DEFINED QM35_IMAGE_PATH)
    set(ext_fw "${QM35_IMAGE_PATH}")
  else()
    set(ext_fw "${ZEPHYR_NRFCONNECT_SDK_QORVO_MODULE_DIR}/firmware/qm35825.bin")
  endif()

  if(DEFINED QM35_IMAGE_VERSION)
    set(ext_fw_version "${QM35_IMAGE_VERSION}")
  else()
    find_package(Python3 REQUIRED COMPONENTS Interpreter)
    set(version_mapping_script
        ${CMAKE_CURRENT_LIST_DIR}/src/aliro/platform/uwb_impl/uwb_qm35_impl/scripts/map_qm35_version.py)
    execute_process(COMMAND python3 ${version_mapping_script} ${ext_fw} OUTPUT_VARIABLE ext_fw_version OUTPUT_STRIP_TRAILING_WHITESPACE)
  endif()

  include(${ZEPHYR_NRF_MODULE_DIR}/cmake/dfu_extra.cmake)
  add_custom_target(ext_fw_target DEPENDS ${ext_fw})
  dfu_extra_add_binary(
    BINARY_PATH ${ext_fw}
    NAME "qm35_fw"
    VERSION ${ext_fw_version}
    DEPENDS ext_fw_target
  )

endif()
