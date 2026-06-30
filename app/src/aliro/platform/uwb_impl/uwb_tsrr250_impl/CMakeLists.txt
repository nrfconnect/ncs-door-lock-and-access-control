#
# Copyright (c) 2026 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

zephyr_include_directories(.)

zephyr_library_sources(uwb_impl.cpp)

if(CONFIG_NXP_UWB_ZEPHYR_MODULE)
  set(NXP_UWB_FTR_INCLUDE_DIR
    ${ZEPHYR_NXP_UWBIOT_MODULE_DIR}/demos/common/demo_csa_controlee/zephyr
  )
  set(NXP_UWB_VIRGO_RW61X_INIT
    ${ZEPHYR_NXP_UWBIOT_MODULE_DIR}/boards/Host/nxp/virgo/virgo/init.c
  )
  set(NXP_UWB_QUEUE_ZEPHYR
    ${ZEPHYR_NXP_UWBIOT_MODULE_DIR}/libs/halimpl/osal/phOsalUwb_Queue_Zephyr.c
  )
  set(NXP_UWB_THREAD_ZEPHYR
    ${ZEPHYR_NXP_UWBIOT_MODULE_DIR}/libs/halimpl/osal/phOsalUwb_Thread_Zephyr.c
  )
  set(NXP_UWB_ADAPTATION
    ${ZEPHYR_NXP_UWBIOT_MODULE_DIR}/libs/uwb-iot/uwb_core/adaptation/UwbAdaptation.c
  )
  set(NXP_UWB_TASK
    ${ZEPHYR_NXP_UWBIOT_MODULE_DIR}/libs/uci-core/src/uwb_task.c
  )
  set(NXP_UWA_DM_ACT
    ${ZEPHYR_NXP_UWBIOT_MODULE_DIR}/libs/uci-core/src/uwa_dm_act.c
  )
  set(NXP_UWB_BUS_INTERFACE
    ${ZEPHYR_NXP_UWBIOT_MODULE_DIR}/boards/Host/nxp/virgo/uwb_bus_interface.c
  )
  set(NXP_UWB_BUS_IO
    ${ZEPHYR_NXP_UWBIOT_MODULE_DIR}/boards/Host/nxp/virgo/uwb_bus_io.c
  )

  if(TARGET zephyr)
    get_target_property(ZEPHYR_SOURCES zephyr SOURCES)
    if(ZEPHYR_SOURCES)
      list(FILTER ZEPHYR_SOURCES EXCLUDE REGEX ".*/boards/Host/nxp/virgo/virgo/init\\.c$")
      list(REMOVE_ITEM ZEPHYR_SOURCES ${NXP_UWB_VIRGO_RW61X_INIT})
      set_target_properties(zephyr PROPERTIES SOURCES "${ZEPHYR_SOURCES}")
    endif()
  endif()

  zephyr_include_directories(${NXP_UWB_FTR_INCLUDE_DIR})
  zephyr_library_link_libraries(uwb_api)
  zephyr_library_link_libraries(halimpl)
  zephyr_library_link_libraries(uci_core)
  zephyr_library_link_libraries(uwb_core)

  foreach(nxp_uwb_target uwb_api halimpl uci_core uwb_core)
    if(TARGET ${nxp_uwb_target})
      target_include_directories(${nxp_uwb_target} PUBLIC ${NXP_UWB_FTR_INCLUDE_DIR})
    endif()
  endforeach()

  if(TARGET halimpl)
    get_target_property(HALIMPL_SOURCES halimpl SOURCES)
    if(HALIMPL_SOURCES)
      list(FILTER HALIMPL_SOURCES EXCLUDE REGEX ".*/libs/halimpl/osal/phOsalUwb_Queue_Zephyr\\.c$")
      list(FILTER HALIMPL_SOURCES EXCLUDE REGEX ".*/libs/halimpl/osal/phOsalUwb_Thread_Zephyr\\.c$")
      list(REMOVE_ITEM HALIMPL_SOURCES ${NXP_UWB_QUEUE_ZEPHYR})
      list(REMOVE_ITEM HALIMPL_SOURCES ${NXP_UWB_THREAD_ZEPHYR})
      set_target_properties(halimpl PROPERTIES SOURCES "${HALIMPL_SOURCES}")
    endif()

    target_sources(halimpl PRIVATE
      ${CMAKE_CURRENT_SOURCE_DIR}/nxp_overrides/phOsalUwb_Queue_Zephyr.c
      ${CMAKE_CURRENT_SOURCE_DIR}/nxp_overrides/phOsalUwb_Thread_Zephyr.c
    )
  endif()

  if(TARGET uwb_core)
    get_target_property(UWB_CORE_SOURCES uwb_core SOURCES)
    if(UWB_CORE_SOURCES)
      list(FILTER UWB_CORE_SOURCES EXCLUDE REGEX ".*/libs/uwb-iot/uwb_core/adaptation/UwbAdaptation\\.c$")
      list(REMOVE_ITEM UWB_CORE_SOURCES ${NXP_UWB_ADAPTATION})
      set_target_properties(uwb_core PROPERTIES SOURCES "${UWB_CORE_SOURCES}")
    endif()

    target_sources(uwb_core PRIVATE
      ${CMAKE_CURRENT_SOURCE_DIR}/nxp_overrides/UwbAdaptation.c
    )
  endif()

  if(TARGET uci_core)
    get_target_property(UCI_CORE_SOURCES uci_core SOURCES)
    if(UCI_CORE_SOURCES)
      list(FILTER UCI_CORE_SOURCES EXCLUDE REGEX ".*/libs/uci-core/src/uwb_task\\.c$")
      list(FILTER UCI_CORE_SOURCES EXCLUDE REGEX ".*/libs/uci-core/src/uwa_dm_act\\.c$")
      list(REMOVE_ITEM UCI_CORE_SOURCES ${NXP_UWB_TASK})
      list(REMOVE_ITEM UCI_CORE_SOURCES ${NXP_UWA_DM_ACT})
      set_target_properties(uci_core PROPERTIES SOURCES "${UCI_CORE_SOURCES}")
    endif()

    target_sources(uci_core PRIVATE
      ${CMAKE_CURRENT_SOURCE_DIR}/nxp_overrides/uwb_task.c
      ${CMAKE_CURRENT_SOURCE_DIR}/nxp_overrides/uwa_dm_act.c
    )
  endif()

  if(TARGET virgo)
    get_target_property(VIRGO_SOURCES virgo SOURCES)
    if(VIRGO_SOURCES)
      list(FILTER VIRGO_SOURCES EXCLUDE REGEX ".*/boards/Host/nxp/virgo/uwb_bus_interface\\.c$")
      list(FILTER VIRGO_SOURCES EXCLUDE REGEX ".*/boards/Host/nxp/virgo/uwb_bus_io\\.c$")
      list(REMOVE_ITEM VIRGO_SOURCES ${NXP_UWB_BUS_INTERFACE})
      list(REMOVE_ITEM VIRGO_SOURCES ${NXP_UWB_BUS_IO})
      set_target_properties(virgo PROPERTIES SOURCES "${VIRGO_SOURCES}")
    endif()

    target_sources(virgo PRIVATE
      ${CMAKE_CURRENT_SOURCE_DIR}/nxp_overrides/uwb_bus_interface.c
      ${CMAKE_CURRENT_SOURCE_DIR}/nxp_overrides/uwb_bus_io.c
    )
  endif()
endif()
