/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "rfal_nfc_config.h"

void rfalNfcWakeupConfig(rfalNfcDiscoverParam *conf)
{
#ifdef CONFIG_RFAL_FEATURE_WAKEUP_MODE

	conf->wakeupEnabled = true;

#ifdef CONFIG_ST25R200_DRV

	conf->wakeupConfigDefault = false;
	conf->wakeupConfig = (rfalWakeUpConfig){
		.period = RFAL_WUM_PERIOD_620MS,
		.irqTout = false,
		.skipCal = false,
		.skipReCal = false,
		.delCal = true,
		.delRef = true,
		.autoAvg = true,
		.measFil = RFAL_WUM_MEAS_FIL_SLOW,
		.measDur = RFAL_WUM_MEAS_DUR_44_28,

		.I.enabled = true,
		.Q.enabled = true,

		.I.delta = 2U,
		.I.reference = RFAL_WUM_REFERENCE_AUTO,
		.I.threshold = ((uint8_t)RFAL_WUM_TRE_ABOVE),
		.I.aaWeight = RFAL_WUM_AA_WEIGHT_32,
		.I.aaInclMeas = true,

		.Q.delta = 4U,
		.Q.reference = RFAL_WUM_REFERENCE_AUTO,
		.Q.threshold = ((uint8_t)RFAL_WUM_TRE_BELOW),
		.Q.aaWeight = RFAL_WUM_AA_WEIGHT_32,
		.Q.aaInclMeas = true,
	};

#endif // CONFIG_ST25R200_DRV
#endif // CONFIG_RFAL_FEATURE_WAKEUP_MODE
}
