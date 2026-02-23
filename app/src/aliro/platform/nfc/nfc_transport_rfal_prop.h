/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <rfal_nfc.h>
#include <rfal_rf.h>

/**
 * @brief Initializes the proprietary NFC technology module.
 *
 * This function must be called once during NFC transport initialization.
 * It prepares the module for operation, including setting up internal state
 * and resources required for proprietary technology detection.
 */
void NfcPropInit(void);

/**
 * @brief Retrieves the RFAL proprietary technology callbacks.
 *
 * This function returns a pointer to the callback structure that should be
 * registered with RFAL to enable proprietary NFC technology support.
 * The returned callbacks integrate seamlessly with RFAL's discovery process.
 *
 * @return Pointer to the RFAL proprietary technology callbacks structure,
 *         or NULL if the module is disabled.
 */
const rfalNfcPropCallbacks *NfcPropGetCallbacks(void);

#ifdef __cplusplus
}
#endif
