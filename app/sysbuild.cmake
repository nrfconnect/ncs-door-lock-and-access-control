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
