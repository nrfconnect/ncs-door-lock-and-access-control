/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "rfal_nfc_config.h"
#include "rfal_wum_common.h"

#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(pal_nfc_config, CONFIG_NFC_LOG_LEVEL);

void rfalNfcWakeupConfig(rfalNfcDiscoverParam *conf)
{
#ifdef CONFIG_RFAL_FEATURE_WAKEUP_MODE

	conf->wakeupEnabled = true;
	conf->wakeupPollBefore = IS_ENABLED(CONFIG_RFAL_WAKEUP_POLL_BEFORE);
	conf->wakeupNPolls = CONFIG_RFAL_WAKEUP_NPOLLS;

#ifdef CONFIG_RFAL_WAKE_UP_MODE_DEFAULT

	conf->wakeupConfigDefault = true;

#else // CONFIG_RFAL_WAKE_UP_MODE_DEFAULT

	conf->wakeupConfigDefault = false;

	rfalWakeUpConfig wakeupConfig = RFAL_WUM_CONFIG_BASE_INIT;

#if defined(CONFIG_RFAL_WAKE_UP_MODE_STRICT)

	wakeupConfig.period = RFAL_WUM_PERIOD_500MS;

	wakeupConfig.I.delta = 2U;
	wakeupConfig.I.threshold = ((uint8_t)RFAL_WUM_TRE_ABOVE);
	wakeupConfig.I.aaWeight = RFAL_WUM_AA_WEIGHT_32;

	wakeupConfig.Q.delta = 4U;
	wakeupConfig.Q.threshold = ((uint8_t)RFAL_WUM_TRE_BELOW);
	wakeupConfig.Q.aaWeight = RFAL_WUM_AA_WEIGHT_32;

	conf->wakeupConfig = wakeupConfig;

#elif defined(CONFIG_RFAL_WAKE_UP_MODE_RELAXED)

	wakeupConfig.period = RFAL_WUM_PERIOD_300MS;

	/* ST25R500 is more sensitive - use higher thresholds to avoid false wakeups */
	wakeupConfig.I.delta = 6U;
	wakeupConfig.I.threshold = ((uint8_t)RFAL_WUM_TRE_ABOVE | (uint8_t)RFAL_WUM_TRE_BELOW);
	wakeupConfig.I.aaWeight = RFAL_WUM_AA_WEIGHT_64;

	wakeupConfig.Q.delta = 5U;
	wakeupConfig.Q.threshold = ((uint8_t)RFAL_WUM_TRE_ABOVE | (uint8_t)RFAL_WUM_TRE_BELOW);
	wakeupConfig.Q.aaWeight = RFAL_WUM_AA_WEIGHT_64;

	conf->wakeupConfig = wakeupConfig;

#else // CONFIG_RFAL_WAKE_UP_MODE_STRICT

	LOG_ERR("Invalid RFAL wake-up mode configuration");

#endif // CONFIG_RFAL_WAKE_UP_MODE_STRICT

#endif // CONFIG_RFAL_WAKE_UP_MODE_DEFAULT

#endif // CONFIG_RFAL_FEATURE_WAKEUP_MODE
}
