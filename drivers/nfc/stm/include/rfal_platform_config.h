/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef RFAL_PLATFORM_CONFIG_H
#define RFAL_PLATFORM_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

/*
******************************************************************************
* RFAL FEATURES CONFIGURATION
******************************************************************************
*/

#ifdef CONFIG_RFAL_FEATURE_LISTEN_MODE
#define RFAL_FEATURE_LISTEN_MODE               true
#else
#define RFAL_FEATURE_LISTEN_MODE               false
#endif // CONFIG_RFAL_FEATURE_LISTEN_MODE       /*!< Enable/Disable RFAL support for Listen Mode */

#ifdef CONFIG_RFAL_FEATURE_WAKEUP_MODE
#define RFAL_FEATURE_WAKEUP_MODE               true
#else
#define RFAL_FEATURE_WAKEUP_MODE               false
#endif // CONFIG_RFAL_FEATURE_WAKEUP_MODE       /*!< Enable/Disable RFAL support for the Wake-Up mode */

#ifdef CONFIG_RFAL_FEATURE_LOWPOWER_MODE
#define RFAL_FEATURE_LOWPOWER_MODE             true
#else
#define RFAL_FEATURE_LOWPOWER_MODE             false
#endif // CONFIG_RFAL_FEATURE_LOWPOWER_MODE     /*!< Enable/Disable RFAL support for the Low Power mode */

#ifdef CONFIG_RFAL_FEATURE_NFCA
#define RFAL_FEATURE_NFCA                      true
#else
#define RFAL_FEATURE_NFCA                      false
#endif // CONFIG_RFAL_FEATURE_NFCA              /*!< Enable/Disable RFAL support for NFC-A (ISO14443A) */

#ifdef CONFIG_RFAL_FEATURE_NFCB
#define RFAL_FEATURE_NFCB                      true
#else
#define RFAL_FEATURE_NFCB                      false
#endif // CONFIG_RFAL_FEATURE_NFCB              /*!< Enable/Disable RFAL support for NFC-B (ISO14443B) */

#ifdef CONFIG_RFAL_FEATURE_NFCF
#define RFAL_FEATURE_NFCF                      true
#else
#define RFAL_FEATURE_NFCF                      false
#endif // CONFIG_RFAL_FEATURE_NFCF              /*!< Enable/Disable RFAL support for NFC-F (FeliCa) */

#ifdef CONFIG_RFAL_FEATURE_NFCV
#define RFAL_FEATURE_NFCV                      true
#else
#define RFAL_FEATURE_NFCV                      false
#endif // CONFIG_RFAL_FEATURE_NFCV              /*!< Enable/Disable RFAL support for NFC-V (ISO15693) */

#ifdef CONFIG_RFAL_FEATURE_T1T
#define RFAL_FEATURE_T1T                       true
#else
#define RFAL_FEATURE_T1T                       false
#endif // CONFIG_RFAL_FEATURE_T1T               /*!< Enable/Disable RFAL support for T1T (Topaz) */

#ifdef CONFIG_RFAL_FEATURE_T2T
#define RFAL_FEATURE_T2T                       true
#else
#define RFAL_FEATURE_T2T                       false
#endif // CONFIG_RFAL_FEATURE_T2T               /*!< Enable/Disable RFAL support for T2T */

#ifdef CONFIG_RFAL_FEATURE_T4T
#define RFAL_FEATURE_T4T                       true
#else
#define RFAL_FEATURE_T4T                       false
#endif // CONFIG_RFAL_FEATURE_T4T               /*!< Enable/Disable RFAL support for T4T */

#ifdef CONFIG_RFAL_FEATURE_ST25TB
#define RFAL_FEATURE_ST25TB                    true
#else
#define RFAL_FEATURE_ST25TB                    false
#endif // CONFIG_RFAL_FEATURE_ST25TB            /*!< Enable/Disable RFAL support for ST25TB */

#ifdef CONFIG_RFAL_FEATURE_ST25xV
#define RFAL_FEATURE_ST25xV                    true
#else
#define RFAL_FEATURE_ST25xV                    false
#endif // CONFIG_RFAL_FEATURE_ST25xV            /*!< Enable/Disable RFAL support for ST25TV/ST25DV */

#ifdef CONFIG_RFAL_FEATURE_DYNAMIC_ANALOG_CONFIG
#define RFAL_FEATURE_DYNAMIC_ANALOG_CONFIG     true
#else
#define RFAL_FEATURE_DYNAMIC_ANALOG_CONFIG     false
#endif // CONFIG_RFAL_FEATURE_DYNAMIC_ANALOG_CONFIG /*!< Enable/Disable Analog Configs to be dynamically updated (RAM) */

#ifdef CONFIG_RFAL_FEATURE_DPO
#define RFAL_FEATURE_DPO                       true
#else
#define RFAL_FEATURE_DPO                       false
#endif // CONFIG_RFAL_FEATURE_DPO               /*!< Enable/Disable RFAL Dynamic Power Output support */

#ifdef CONFIG_RFAL_FEATURE_ISO_DEP
#define RFAL_FEATURE_ISO_DEP                   true
#else
#define RFAL_FEATURE_ISO_DEP                   false
#endif // CONFIG_RFAL_FEATURE_ISO_DEP           /*!< Enable/Disable RFAL support for ISO-DEP (ISO14443-4) */

#ifdef CONFIG_RFAL_FEATURE_ISO_DEP_POLL
#define RFAL_FEATURE_ISO_DEP_POLL              true
#else
#define RFAL_FEATURE_ISO_DEP_POLL              false
#endif // CONFIG_RFAL_FEATURE_ISO_DEP_POLL      /*!< Enable/Disable RFAL support for Poller mode (PCD) ISO-DEP (ISO14443-4) */

#ifdef CONFIG_RFAL_FEATURE_ISO_DEP_LISTEN
#define RFAL_FEATURE_ISO_DEP_LISTEN            true
#else
#define RFAL_FEATURE_ISO_DEP_LISTEN            false
#endif // CONFIG_RFAL_FEATURE                   /*!< Enable/Disable RFAL support for Listen mode (PICC) ISO-DEP (ISO14443-4) */

#ifdef CONFIG_RFAL_FEATURE_NFC_DEP
#define RFAL_FEATURE_NFC_DEP                   true
#else
#define RFAL_FEATURE_NFC_DEP                   false
#endif // CONFIG_RFAL_FEATURE_NFC_DEP           /*!< Enable/Disable RFAL support for NFC-DEP (NFCIP1/P2P) */



#define RFAL_FEATURE_ISO_DEP_IBLOCK_MAX_LEN    CONFIG_RFAL_FEATURE_ISO_DEP_IBLOCK_MAX_LEN   /*!< ISO-DEP I-Block max length. Please use values as defined by rfalIsoDepFSx */
#define RFAL_FEATURE_NFC_DEP_BLOCK_MAX_LEN     CONFIG_RFAL_FEATURE_NFC_DEP_BLOCK_MAX_LEN    /*!< NFC-DEP Block/Payload length. Allowed values: 64, 128, 192, 254           */
#define RFAL_FEATURE_NFC_RF_BUF_LEN            CONFIG_RFAL_FEATURE_NFC_RF_BUF_LEN           /*!< RF buffer length used by RFAL NFC layer                                   */

#define RFAL_FEATURE_ISO_DEP_APDU_MAX_LEN      CONFIG_RFAL_FEATURE_ISO_DEP_APDU_MAX_LEN      /*!< ISO-DEP APDU max length. Please use multiples of I-Block max length       */
#define RFAL_FEATURE_NFC_DEP_PDU_MAX_LEN       CONFIG_RFAL_FEATURE_NFC_DEP_PDU_MAX_LEN       /*!< NFC-DEP PDU max length.                                                   */

#ifdef __cplusplus
}
#endif

#endif /* RFAL_PLATFORM_CONFIG_H */
