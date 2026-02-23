/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef RFAL_WUM_COMMON_H_
#define RFAL_WUM_COMMON_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Base wake-up configuration initializer
 *
 * Common settings shared between ST25R200 and ST25R500.
 * Chip-specific settings (period, delta, threshold, aaWeight)
 * must be set after using this initializer.
 */

// clang-format off
#define RFAL_WUM_CONFIG_BASE_INIT                   \
	{                                           \
		.irqTout = false,                   \
		.skipCal = false,                   \
		.skipReCal = false,                 \
		.delCal = true,                     \
		.delRef = true,                     \
		.autoAvg = true,                    \
		.measFil = RFAL_WUM_MEAS_FIL_SLOW,  \
		.measDur = RFAL_WUM_MEAS_DUR_44_28, \
		.I.enabled = true,                  \
		.Q.enabled = true,                  \
		.I.reference = RFAL_WUM_REFERENCE_AUTO, \
		.Q.reference = RFAL_WUM_REFERENCE_AUTO, \
		.I.aaInclMeas = true,               \
		.Q.aaInclMeas = true,               \
	}
// clang-format on

#ifdef __cplusplus
}
#endif

#endif /* RFAL_WUM_COMMON_H_ */
