#
# Copyright (c) 2025 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

if("matter" IN_LIST SNIPPET)
  if(SB_CONFIG_SOC_SERIES_NRF52X)
    set(PM_STATIC_YML_FILE ${CMAKE_CURRENT_LIST_DIR}/pm_static_nrf52840dk_nrf52840_matter.yml CACHE INTERNAL "")
  elseif(SB_CONFIG_SOC_SERIES_NRF53X)
    set(PM_STATIC_YML_FILE ${CMAKE_CURRENT_LIST_DIR}/pm_static_nrf5340dk_nrf5340_cpuapp_matter.yml CACHE INTERNAL "")
  elseif(SB_CONFIG_SOC_SERIES_NRF54LX)
    set(PM_STATIC_YML_FILE ${CMAKE_CURRENT_LIST_DIR}/pm_static_nrf54l15dk_nrf54l15_cpuapp_matter.yml CACHE INTERNAL "")
  else()
    message(FATAL_ERROR "Unsupported SOC")
  endif()

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
