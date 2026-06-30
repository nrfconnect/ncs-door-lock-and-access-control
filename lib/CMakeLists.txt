#
# Copyright (c) 2025 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

if(CONFIG_SOC_SERIES_NRF52X)
  set(SOC_NAME "nrf52840")
elseif(CONFIG_SOC_SERIES_NRF53X)
  set(SOC_NAME "nrf5340")
elseif(CONFIG_SOC_NRF54L15)
  set(SOC_NAME "nrf54l15")
elseif(CONFIG_SOC_NRF54LM20A)
  set(SOC_NAME "nrf54lm20a")
endif()

add_subdirectory_ifdef(CONFIG_NCS_ALIRO aliro)
if(CONFIG_QM35_UWB_LIB OR CONFIG_TSRR250_UWB_ALIRO_ZEPHYR)
  add_subdirectory(qm35_uwb)
endif()
