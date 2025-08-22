/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "rfal_nfc_config.h"
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(pal_nfc_config, CONFIG_NFC_LOG_LEVEL);

void rfalNfcWakeupConfig(rfalNfcDiscoverParam *conf)
{
#ifdef CONFIG_RFAL_FEATURE_WAKEUP_MODE

	conf->wakeupEnabled = true;

#ifdef CONFIG_ST25R200_DRV

#ifdef CONFIG_RFAL_WAKE_UP_MODE_DEFAULT

	conf->wakeupConfigDefault = true;

#else // CONFIG_RFAL_WAKE_UP_MODE_DEFAULT

	conf->wakeupConfigDefault = false;

	rfalWakeUpConfig wakeupConfig = {

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

		.I.reference = RFAL_WUM_REFERENCE_AUTO,
		.Q.reference = RFAL_WUM_REFERENCE_AUTO,

		.I.aaInclMeas = true,
		.Q.aaInclMeas = true,

	};

#if defined(CONFIG_RFAL_WAKE_UP_MODE_STRICT)

	wakeupConfig.period = RFAL_WUM_PERIOD_620MS;

	wakeupConfig.I.delta = 2U;
	wakeupConfig.I.threshold = ((uint8_t)RFAL_WUM_TRE_ABOVE);
	wakeupConfig.I.aaWeight = RFAL_WUM_AA_WEIGHT_32;

	wakeupConfig.Q.delta = 4U;
	wakeupConfig.Q.threshold = ((uint8_t)RFAL_WUM_TRE_BELOW);
	wakeupConfig.Q.aaWeight = RFAL_WUM_AA_WEIGHT_32;

	conf->wakeupConfig = wakeupConfig;

#elif defined(CONFIG_RFAL_WAKE_UP_MODE_RELAXED)

	wakeupConfig.period = RFAL_WUM_PERIOD_310MS;

	wakeupConfig.I.delta = 2U;
	wakeupConfig.I.threshold = ((uint8_t)RFAL_WUM_TRE_ABOVE | (uint8_t)RFAL_WUM_TRE_BELOW);
	wakeupConfig.I.aaWeight = RFAL_WUM_AA_WEIGHT_16;

	wakeupConfig.Q.delta = 1U;
	wakeupConfig.Q.threshold = ((uint8_t)RFAL_WUM_TRE_ABOVE | (uint8_t)RFAL_WUM_TRE_BELOW);
	wakeupConfig.Q.aaWeight = RFAL_WUM_AA_WEIGHT_16;

	conf->wakeupConfig = wakeupConfig;

#else // CONFIG_RFAL_WAKE_UP_MODE_STRICT

	LOG_ERR("Invalid RFAL wake-up mode configuration");

#endif // CONFIG_RFAL_WAKE_UP_MODE_STRICT

#endif // CONFIG_RFAL_WAKE_UP_MODE_DEFAULT
#endif // CONFIG_ST25R200_DRV
#endif // CONFIG_RFAL_FEATURE_WAKEUP_MODE
}
