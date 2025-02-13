/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef RFAL_NFC_CONFIG_H
#define RFAL_NFC_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "rfal_nfc.h"

/**
 * @brief Enables wake-up mode. Additionally, the function configures non-default wake-up mode for the
 * ST25R200/100 NFC integrated circuit (IC). The example configuration sets the NFC IC into card approch
 * only, according to Application Note: AN5993.
 * NOTE: This is only example configuration, the final configuration MUST be detuned for specific product.
 *
 * @param conf [input] pointer to discovery configuration parameters.
 */
void rfalNfcWakeupConfig(rfalNfcDiscoverParam *conf);

#ifdef __cplusplus
}
#endif

#endif /* RFAL_NFC_CONFIG_H */
