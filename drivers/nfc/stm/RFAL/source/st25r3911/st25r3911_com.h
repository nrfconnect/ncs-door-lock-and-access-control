
/******************************************************************************
  * @attention
  *
  * COPYRIGHT 2016 STMicroelectronics, all rights reserved
  *
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an "AS IS" BASIS,
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied,
  * AND SPECIFICALLY DISCLAIMING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
  * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
******************************************************************************/



/*
 *      PROJECT:   ST25R3911 firmware
 *      Revision:
 *      LANGUAGE:  ISO C99
 */

/*! \file
 *
 *  \author Ulrich Herrmann
 *
 *  \brief ST25R3911 communication declaration file
 *
 */
/*!
 * This driver provides basic abstraction for communication with the ST25R3911.
 * It uses the SPI driver for interfacing with the ST25R3911.
 *
 * API:
 * - Read Register: #st25r3911ReadRegister
 * - Modify Register: #st25r3911ModifyRegister
 * - Write Register: #st25r3911WriteRegister
 * - Write Multiple Registers: #st25r3911WriteMultipleRegisters
 * - Load ST25R3911 FIFO with data: #st25r3911WriteFifo
 * - Read from ST25R3911 FIFO: #st25r3911ReadFifo
 * - Execute direct command: #st25r3911ExecuteCommand
 * 
 *
 * \addtogroup RFAL
 * @{
 *
 * \addtogroup RFAL-HAL
 * \brief RFAL Hardware Abstraction Layer
 * @{
 *
 * \addtogroup ST25R3911
 * \brief RFAL ST25R3911 Driver
 * @{
 * 
 * \addtogroup ST25R3911_Com
 * \brief RFAL ST25R3911 Communication
 * @{
 * 
 */

#ifndef ST25R3911_COM_H
#define ST25R3911_COM_H

/*
******************************************************************************
* INCLUDES
******************************************************************************
*/
#include "rfal_platform.h"
#include "rfal_utils.h"

/*
******************************************************************************
* GLOBAL DEFINES
******************************************************************************
*/

#define ST25R3911_FIFO_STATUS_LEN                  2           /*!< Number of FIFO Status Register */




#define ST25R3911_REG_IO_CONF1                     0x00U        /*!< RW IO Configuration Register 1 */
#define ST25R3911_REG_IO_CONF2                     0x01U        /*!< RW IO Configuration Register 2 */

#define ST25R3911_REG_OP_CONTROL                   0x02U        /*!< RW Operation Control Register */
#define ST25R3911_REG_MODE                         0x03U        /*!< RW Mode Definition Register */
#define ST25R3911_REG_BIT_RATE                     0x04U        /*!< RW Bit Rate Definition Register */

#define ST25R3911_REG_ISO14443A_NFC                0x05U        /*!< RW ISO14443A and NFC 106 kBit/s Settings Register */
#define ST25R3911_REG_ISO14443B_1                  0x06U        /*!< RW ISO14443B Settings Register 1 */
#define ST25R3911_REG_ISO14443B_2                  0x07U        /*!< RW ISO14443B Settings Register 2 */
#define ST25R3911_REG_STREAM_MODE                  0x08U        /*!< RW Stream Mode Definition Register */
#define ST25R3911_REG_AUX                          0x09U        /*!< RW Auxiliary Definition Register */
#define ST25R3911_REG_RX_CONF1                     0x0AU        /*!< RW Receiver Configuration Register 1 */
#define ST25R3911_REG_RX_CONF2                     0x0BU        /*!< RW Receiver Configuration Register 2 */
#define ST25R3911_REG_RX_CONF3                     0x0CU        /*!< RW Receiver Configuration Register 3 */
#define ST25R3911_REG_RX_CONF4                     0x0DU        /*!< RW Receiver Configuration Register 4 */

#define ST25R3911_REG_MASK_RX_TIMER                0x0EU        /*!< RW Mask Receive Timer Register */
#define ST25R3911_REG_NO_RESPONSE_TIMER1           0x0FU        /*!< RW No-response Timer Register 1 */
#define ST25R3911_REG_NO_RESPONSE_TIMER2           0x10U        /*!< RW No-response Timer Register 2 */
#define ST25R3911_REG_GPT_CONTROL                  0x11U        /*!< RW General Purpose Timer Control Register */
#define ST25R3911_REG_GPT1                         0x12U        /*!< RW General Purpose Timer Register 1 */
#define ST25R3911_REG_GPT2                         0x13U        /*!< RW General Purpose Timer Register 2 */

#define ST25R3911_REG_IRQ_MASK_MAIN                0x14U        /*!< RW Mask Main Interrupt Register */
#define ST25R3911_REG_IRQ_MASK_TIMER_NFC           0x15U        /*!< RW Mask Timer and NFC Interrupt Register */
#define ST25R3911_REG_IRQ_MASK_ERROR_WUP           0x16U        /*!< RW Mask Error and Wake-up Interrupt Register */
#define ST25R3911_REG_IRQ_MAIN                     0x17U        /*!< R  Main Interrupt Register */
#define ST25R3911_REG_IRQ_TIMER_NFC                0x18U        /*!< R  Timer and NFC Interrupt Register */
#define ST25R3911_REG_IRQ_ERROR_WUP                0x19U        /*!< R  Error and Wake-up Interrupt Register */
#define ST25R3911_REG_FIFO_RX_STATUS1              0x1AU        /*!< R  FIFO RX Status Register 1 */
#define ST25R3911_REG_FIFO_RX_STATUS2              0x1BU        /*!< R  FIFO RX Status Register 2 */
#define ST25R3911_REG_COLLISION_STATUS             0x1CU        /*!< R  Collision Display Register */

#define ST25R3911_REG_NUM_TX_BYTES1                0x1DU        /*!< RW Number of Transmitted Bytes Register 1 */
#define ST25R3911_REG_NUM_TX_BYTES2                0x1EU        /*!< RW Number of Transmitted Bytes Register 2 */

#define ST25R3911_REG_NFCIP1_BIT_RATE              0x1FU        /*!< R  NFCIP Bit Rate Detection Display Register */

#define ST25R3911_REG_AD_RESULT                    0x20U        /*!< R  A/D Converter Output Register */

#define ST25R3911_REG_ANT_CAL_CONTROL              0x21U        /*!< RW Antenna Calibration Control Register */
#define ST25R3911_REG_ANT_CAL_TARGET               0x22U        /*!< RW Antenna Calibration Target Register */
#define ST25R3911_REG_ANT_CAL_RESULT               0x23U        /*!< R  Antenna Calibration Display Register */

#define ST25R3911_REG_AM_MOD_DEPTH_CONTROL         0x24U        /*!< RW AM Modulation Depth Control Register */
#define ST25R3911_REG_AM_MOD_DEPTH_RESULT          0x25U        /*!< R  AM Modulation Depth Display Register */
#define ST25R3911_REG_RFO_AM_ON_LEVEL              0x26U        /*!< RW RFO AM Modulation (On) Level Definition Register */
#define ST25R3911_REG_RFO_AM_OFF_LEVEL             0x27U        /*!< RW RFO Normal (AM Off) Level Definition Register */

#define ST25R3911_REG_FIELD_THRESHOLD              0x29U        /*!< RW External Field Detector Threshold Register */

#define ST25R3911_REG_REGULATOR_CONTROL            0x2AU        /*!< RW Regulated Voltage Control Register */
#define ST25R3911_REG_REGULATOR_RESULT             0x2BU        /*!< R Regulator Display Register */

#define ST25R3911_REG_RSSI_RESULT                  0x2CU        /*!< R RSSI Display Register*/
#define ST25R3911_REG_GAIN_RED_STATE               0x2DU        /*!< R Gain Reduction State Register*/

#define ST25R3911_REG_CAP_SENSOR_CONTROL           0x2EU        /*!< RW Capacitive Sensor Control Register */
#define ST25R3911_REG_CAP_SENSOR_RESULT            0x2FU        /*!< R  Capacitive Sensor Display Register */

#define ST25R3911_REG_AUX_DISPLAY                  0x30U        /*!< R Auxiliary Display Register */

#define ST25R3911_REG_WUP_TIMER_CONTROL            0x31U        /*!< RW Wake-up Timer Control Register */
#define ST25R3911_REG_AMPLITUDE_MEASURE_CONF       0x32U        /*!< RW Amplitude Measurement Configuration Register */
#define ST25R3911_REG_AMPLITUDE_MEASURE_REF        0x33U        /*!< RW Amplitude Measurement Reference Register */
#define ST25R3911_REG_AMPLITUDE_MEASURE_AA_RESULT  0x34U        /*!< R  Amplitude Measurement Auto Averaging Display Register */
#define ST25R3911_REG_AMPLITUDE_MEASURE_RESULT     0x35U        /*!< R  Amplitude Measurement Display Register */
#define ST25R3911_REG_PHASE_MEASURE_CONF           0x36U        /*!< RW Phase Measurement Configuration Register */
#define ST25R3911_REG_PHASE_MEASURE_REF            0x37U        /*!< RW Phase Measurement Reference Register */
#define ST25R3911_REG_PHASE_MEASURE_AA_RESULT      0x38U        /*!< R  Phase Measurement Auto Averaging Display Register */
#define ST25R3911_REG_PHASE_MEASURE_RESULT         0x39U        /*!< R  Phase Measurement Display Register */
#define ST25R3911_REG_CAPACITANCE_MEASURE_CONF     0x3AU        /*!< RW Capacitance Measurement Configuration Register */
#define ST25R3911_REG_CAPACITANCE_MEASURE_REF      0x3BU        /*!< RW Capacitance Measurement Reference Register */
#define ST25R3911_REG_CAPACITANCE_MEASURE_AA_RESULT 0x3CU       /*!< R  Capacitance Measurement Auto Averaging Display Register */
#define ST25R3911_REG_CAPACITANCE_MEASURE_RESULT   0x3DU        /*!< R  Capacitance Measurement Display Register */

#define ST25R3911_REG_IC_IDENTITY                  0x3FU        /*!< R  Chip Id: 0 for old silicon, v2 silicon: 0x09 */


/*! Register bit definitions  \cond DOXYGEN_SUPPRESS */

#define ST25R3911_REG_IO_CONF1_lf_clk_off                     (1U<<0)
#define ST25R3911_REG_IO_CONF1_out_cl0                        (1U<<1)
#define ST25R3911_REG_IO_CONF1_out_cl1                        (1U<<2)
#define ST25R3911_REG_IO_CONF1_mask_out_cl                    (3U<<1)
#define ST25R3911_REG_IO_CONF1_osc                            (1U<<3)
#define ST25R3911_REG_IO_CONF1_fifo_lt                        (1U<<4)
#define ST25R3911_REG_IO_CONF1_fifo_lt_32bytes                (0U<<4)
#define ST25R3911_REG_IO_CONF1_fifo_lt_16bytes                (1U<<4)
#define ST25R3911_REG_IO_CONF1_fifo_lr                        (1U<<5)
#define ST25R3911_REG_IO_CONF1_fifo_lr_64bytes                (0U<<5)
#define ST25R3911_REG_IO_CONF1_fifo_lr_80bytes                (1U<<5)
#define ST25R3911_REG_IO_CONF1_rfo2                           (1U<<6)
#define ST25R3911_REG_IO_CONF1_single                         (1U<<7)
#define ST25R3911_REG_IO_CONF2_slow_up                        (1U<<0)
#define ST25R3911_REG_IO_CONF2_io_18                          (1U<<2)
#define ST25R3911_REG_IO_CONF2_miso_pd1                       (1U<<3)
#define ST25R3911_REG_IO_CONF2_miso_pd2                       (1U<<4)
#define ST25R3911_REG_IO_CONF2_vspd_off                       (1U<<6)
#define ST25R3911_REG_IO_CONF2_sup3V                          (1U<<7)
#define ST25R3911_REG_OP_CONTROL_wu                           (1U<<2)
#define ST25R3911_REG_OP_CONTROL_tx_en                        (1U<<3)
#define ST25R3911_REG_OP_CONTROL_rx_man                       (1U<<4)
#define ST25R3911_REG_OP_CONTROL_rx_chn                       (1U<<5)
#define ST25R3911_REG_OP_CONTROL_rx_en                        (1U<<6)
#define ST25R3911_REG_OP_CONTROL_en                           (1U<<7)
#define ST25R3911_REG_MODE_nfc_ar                             (1U<<0)
#define ST25R3911_REG_MODE_nfc_ar_on                          (1U<<0)
#define ST25R3911_REG_MODE_nfc_ar_off                         (0U<<0)
#define ST25R3911_REG_MODE_mask_om                            (0xfU<<3)
#define ST25R3911_REG_MODE_om_nfc                             (0x0U<<3)
#define ST25R3911_REG_MODE_om_iso14443a                       (0x1U<<3)
#define ST25R3911_REG_MODE_om_iso14443b                       (0x2U<<3)
#define ST25R3911_REG_MODE_om_felica                          (0x3U<<3)
#define ST25R3911_REG_MODE_om_topaz                           (0x4U<<3)
#define ST25R3911_REG_MODE_om_subcarrier_stream               (0xeU<<3)
#define ST25R3911_REG_MODE_om_bpsk_stream                     (0xfU<<3)
#define ST25R3911_REG_MODE_om_bit_rate_detection              (0x0U<<3)
#define ST25R3911_REG_MODE_om_nfcip1_normal_mode              (0x1U<<3)
#define ST25R3911_REG_MODE_targ                               (1U<<7)
#define ST25R3911_REG_MODE_targ_targ                          (1U<<7)
#define ST25R3911_REG_MODE_targ_init                          (0U<<7)
#define ST25R3911_REG_BIT_RATE_mask_txrate                    (0xfU<<4)
#define ST25R3911_REG_BIT_RATE_shift_txrate                   (4U)
#define ST25R3911_REG_BIT_RATE_txrate_106                     (0x0U<<4)
#define ST25R3911_REG_BIT_RATE_txrate_212                     (0x1U<<4)
#define ST25R3911_REG_BIT_RATE_txrate_424                     (0x2U<<4)
#define ST25R3911_REG_BIT_RATE_txrate_848                     (0x3U<<4)
#define ST25R3911_REG_BIT_RATE_txrate_1695                    (0x4U<<4)
#define ST25R3911_REG_BIT_RATE_txrate_3390                    (0x5U<<4)
#define ST25R3911_REG_BIT_RATE_txrate_6780                    (0x6U<<4)
#define ST25R3911_REG_BIT_RATE_mask_rxrate                    (0xfU<<0)
#define ST25R3911_REG_BIT_RATE_shift_rxrate                   (0U)
#define ST25R3911_REG_BIT_RATE_rxrate_106                     (0x0U<<0)
#define ST25R3911_REG_BIT_RATE_rxrate_212                     (0x1U<<0)
#define ST25R3911_REG_BIT_RATE_rxrate_424                     (0x2U<<0)
#define ST25R3911_REG_BIT_RATE_rxrate_848                     (0x3U<<0)
#define ST25R3911_REG_BIT_RATE_rxrate_1695                    (0x4U<<0)
#define ST25R3911_REG_BIT_RATE_rxrate_3390                    (0x5U<<0)
#define ST25R3911_REG_BIT_RATE_rxrate_6780                    (0x6U<<0)
#define ST25R3911_REG_ISO14443A_NFC_antcl                     (1U<<0)
#define ST25R3911_REG_ISO14443A_NFC_mask_p_len                (0xfU<<1)
#define ST25R3911_REG_ISO14443A_NFC_shift_p_len               (1U)
#define ST25R3911_REG_ISO14443A_NFC_nfc_f0                    (1U<<5)
#define ST25R3911_REG_ISO14443A_NFC_nfc_f0_off                (0U<<5)
#define ST25R3911_REG_ISO14443A_NFC_no_rx_par                 (1U<<6)
#define ST25R3911_REG_ISO14443A_NFC_no_rx_par_off             (0U<<6)
#define ST25R3911_REG_ISO14443A_NFC_no_tx_par                 (1U<<7)
#define ST25R3911_REG_ISO14443A_NFC_no_tx_par_off             (0U<<7)
#define ST25R3911_REG_ISO14443B_1_mask_eof                    (1U<<2)
#define ST25R3911_REG_ISO14443B_1_eof_10etu                   (0U<<2)
#define ST25R3911_REG_ISO14443B_1_eof_11etu                   (1U<<2)
#define ST25R3911_REG_ISO14443B_1_mask_sof                    (3U<<3)
#define ST25R3911_REG_ISO14443B_1_mask_sof_0                  (1U<<4)
#define ST25R3911_REG_ISO14443B_1_sof_0_10etu                 (0U<<4)
#define ST25R3911_REG_ISO14443B_1_sof_0_11etu                 (1U<<4)
#define ST25R3911_REG_ISO14443B_1_mask_sof_1                  (1U<<3)
#define ST25R3911_REG_ISO14443B_1_sof_1_2etu                  (0U<<3)
#define ST25R3911_REG_ISO14443B_1_sof_2_3etu                  (1U<<3)
#define ST25R3911_REG_ISO14443B_1_mask_egt                    (7U<<5)
#define ST25R3911_REG_ISO14443B_1_shift_egt                   (5U)
#define ST25R3911_REG_ISO14443B_2_eof_12                      (1U<<3)
#define ST25R3911_REG_ISO14443B_2_eof_12_10to11etu            (0U<<3)
#define ST25R3911_REG_ISO14443B_2_eof_12_10to12etu            (1U<<3)
#define ST25R3911_REG_ISO14443B_2_no_eof                      (1U<<4)
#define ST25R3911_REG_ISO14443B_2_no_sof                      (1U<<5)
#define ST25R3911_REG_ISO14443B_2_mask_tr1                    (3U<<6)
#define ST25R3911_REG_ISO14443B_2_shift_tr1                   (6U)
#define ST25R3911_REG_ISO14443B_2_tr1_0                       (1U<<6)
#define ST25R3911_REG_ISO14443B_2_tr1_1                       (1U<<7)
#define ST25R3911_REG_ISO14443B_2_tr1_80fs80fs                (0U<<6)
#define ST25R3911_REG_ISO14443B_2_tr1_64fs32fs                (1U<<6)
#define ST25R3911_REG_STREAM_MODE_mask_stx                    (7U<<0)
#define ST25R3911_REG_STREAM_MODE_shift_stx                   (0U)
#define ST25R3911_REG_STREAM_MODE_stx_106                     (0U<<0)
#define ST25R3911_REG_STREAM_MODE_stx_212                     (1U<<0)
#define ST25R3911_REG_STREAM_MODE_stx_424                     (2U<<0)
#define ST25R3911_REG_STREAM_MODE_stx_848                     (3U<<0)
#define ST25R3911_REG_STREAM_MODE_stx_1695                    (4U<<0)
#define ST25R3911_REG_STREAM_MODE_stx_3390                    (5U<<0)
#define ST25R3911_REG_STREAM_MODE_stx_6780                    (6U<<0)
#define ST25R3911_REG_STREAM_MODE_mask_scp                    (3U<<3)
#define ST25R3911_REG_STREAM_MODE_shift_scp                   (3U)
#define ST25R3911_REG_STREAM_MODE_scp_1pulse                  (0U<<3)
#define ST25R3911_REG_STREAM_MODE_scp_2pulses                 (1U<<3)
#define ST25R3911_REG_STREAM_MODE_scp_4pulses                 (2U<<3)
#define ST25R3911_REG_STREAM_MODE_scp_8pulses                 (3U<<3)
#define ST25R3911_REG_STREAM_MODE_mask_scf                    (3U<<5)
#define ST25R3911_REG_STREAM_MODE_shift_scf                   (5U)
#define ST25R3911_REG_STREAM_MODE_scf_bpsk848                 (0U<<5)
#define ST25R3911_REG_STREAM_MODE_scf_bpsk1695                (1U<<5)
#define ST25R3911_REG_STREAM_MODE_scf_bpsk3390                (2U<<5)
#define ST25R3911_REG_STREAM_MODE_scf_bpsk106                 (3U<<5)
#define ST25R3911_REG_STREAM_MODE_scf_sc212                   (0U<<5)
#define ST25R3911_REG_STREAM_MODE_scf_sc424                   (1U<<5)
#define ST25R3911_REG_STREAM_MODE_scf_sc848                   (2U<<5)
#define ST25R3911_REG_STREAM_MODE_scf_sc1695                  (3U<<5)
#define ST25R3911_REG_AUX_mask_nfc_n                          (3U<<0)
#define ST25R3911_REG_AUX_nfc_n0                              (1U<<0)
#define ST25R3911_REG_AUX_nfc_n1                              (1U<<1)
#define ST25R3911_REG_AUX_rx_tol                              (1U<<2)
#define ST25R3911_REG_AUX_ook_hr                              (1U<<3)
#define ST25R3911_REG_AUX_en_fd                               (1U<<4)
#define ST25R3911_REG_AUX_tr_am                               (1U<<5)
#define ST25R3911_REG_AUX_crc_2_fifo                          (1U<<6)
#define ST25R3911_REG_AUX_no_crc_rx                           (1U<<7)
#define ST25R3911_REG_RX_CONF1_z12k                           (1U<<0)
#define ST25R3911_REG_RX_CONF1_h80                            (1U<<1)
#define ST25R3911_REG_RX_CONF1_h200                           (1U<<2)
#define ST25R3911_REG_RX_CONF1_mask_lp                        (7U<<3)
#define ST25R3911_REG_RX_CONF1_lp_1200khz                     (0U<<3)
#define ST25R3911_REG_RX_CONF1_lp_600khz                      (1U<<3)
#define ST25R3911_REG_RX_CONF1_lp_300khz                      (2U<<3)
#define ST25R3911_REG_RX_CONF1_lp_2000khz                     (4U<<3)
#define ST25R3911_REG_RX_CONF1_lp_7000khz                     (5U<<3)
#define ST25R3911_REG_RX_CONF1_amd_sel                        (1U<<6)
#define ST25R3911_REG_RX_CONF1_ch_sel                         (1U<<7)
#define ST25R3911_REG_RX_CONF2_sqm_dyn                        (1U<<1)
#define ST25R3911_REG_RX_CONF2_agc_alg                        (1U<<2)
#define ST25R3911_REG_RX_CONF2_agc_m                          (1U<<3)
#define ST25R3911_REG_RX_CONF2_agc_en                         (1U<<4)
#define ST25R3911_REG_RX_CONF2_lf_en                          (1U<<5)
#define ST25R3911_REG_RX_CONF2_lf_op                          (1U<<6)
#define ST25R3911_REG_RX_CONF2_rx_lp                          (1U<<7)
#define ST25R3911_REG_RX_CONF3_rg_nfc                         (1U<<0)
#define ST25R3911_REG_RX_CONF3_lim                            (1U<<1)
#define ST25R3911_REG_RX_CONF3_shift_rg1_pm                   (2U)
#define ST25R3911_REG_RX_CONF3_mask_rg1_pm                    (0x7U<<2)
#define ST25R3911_REG_RX_CONF3_rg1_pm0                        (1U<<2)
#define ST25R3911_REG_RX_CONF3_rg1_pm1                        (1U<<3)
#define ST25R3911_REG_RX_CONF3_rg1_pm2                        (1U<<4)
#define ST25R3911_REG_RX_CONF3_shift_rg1_am                   (5U)
#define ST25R3911_REG_RX_CONF3_mask_rg1_am                    (0x7U<<5)
#define ST25R3911_REG_RX_CONF3_rg1_am0                        (1U<<5)
#define ST25R3911_REG_RX_CONF3_rg1_am1                        (1U<<6)
#define ST25R3911_REG_RX_CONF3_rg1_am2                        (1U<<7)
#define ST25R3911_REG_RX_CONF4_shift_rg2_pm                   (0U)
#define ST25R3911_REG_RX_CONF4_mask_rg2_pm                    (0xfU<<0)
#define ST25R3911_REG_RX_CONF4_rg2_pm0                        (1U<<0)
#define ST25R3911_REG_RX_CONF4_rg2_pm1                        (1U<<1)
#define ST25R3911_REG_RX_CONF4_rg2_pm2                        (1U<<2)
#define ST25R3911_REG_RX_CONF4_rg2_pm3                        (1U<<3)
#define ST25R3911_REG_RX_CONF4_shift_rg2_am                   (4U)
#define ST25R3911_REG_RX_CONF4_mask_rg2_am                    (0xfU<<4)
#define ST25R3911_REG_RX_CONF4_rg2_am0                        (1U<<4)
#define ST25R3911_REG_RX_CONF4_rg2_am1                        (1U<<5)
#define ST25R3911_REG_RX_CONF4_rg2_am2                        (1U<<6)
#define ST25R3911_REG_RX_CONF4_rg2_am3                        (1U<<7)
#define ST25R3911_REG_GPT_CONTROL_nrt_step                    (1U<<0)
#define ST25R3911_REG_GPT_CONTROL_nrt_emv                     (1U<<1)
#define ST25R3911_REG_GPT_CONTROL_gptc0                       (1U<<5)
#define ST25R3911_REG_GPT_CONTROL_gptc1                       (1U<<6)
#define ST25R3911_REG_GPT_CONTROL_gptc2                       (1U<<7)
#define ST25R3911_REG_GPT_CONTROL_gptc_mask                   (0x7U<<5)
#define ST25R3911_REG_GPT_CONTROL_gptc_no_trigger             (0x0U<<5)
#define ST25R3911_REG_GPT_CONTROL_gptc_erx                    (0x1U<<5)
#define ST25R3911_REG_GPT_CONTROL_gptc_srx                    (0x2U<<5)
#define ST25R3911_REG_GPT_CONTROL_gptc_etx_nfc                (0x3U<<5)
#define ST25R3911_REG_FIFO_RX_STATUS2_np_lb                   (1U<<0)
#define ST25R3911_REG_FIFO_RX_STATUS2_mask_fifo_lb            (7U<<1)
#define ST25R3911_REG_FIFO_RX_STATUS2_shift_fifo_lb           (1U)
#define ST25R3911_REG_FIFO_RX_STATUS2_fifo_lb0                (1U<<1)
#define ST25R3911_REG_FIFO_RX_STATUS2_fifo_lb1                (1U<<2)
#define ST25R3911_REG_FIFO_RX_STATUS2_fifo_lb2                (1U<<3)
#define ST25R3911_REG_FIFO_RX_STATUS2_fifo_ncp                (1U<<4)
#define ST25R3911_REG_FIFO_RX_STATUS2_fifo_ovr                (1U<<5)
#define ST25R3911_REG_FIFO_RX_STATUS2_fifo_unf                (1U<<6)
#define ST25R3911_REG_COLLISION_STATUS_c_pb                   (1U<<0)
#define ST25R3911_REG_COLLISION_STATUS_mask_c_bit             (3U<<1)
#define ST25R3911_REG_COLLISION_STATUS_shift_c_bit            (1U)
#define ST25R3911_REG_COLLISION_STATUS_mask_c_byte            (0xfU<<4)
#define ST25R3911_REG_COLLISION_STATUS_shift_c_byte           (4U)
#define ST25R3911_ST25R3911_REG_NFCIP1_BIT_RATE_nfc_rate0     (1U<<4)
#define ST25R3911_ST25R3911_REG_NFCIP1_BIT_RATE_nfc_rate1     (1U<<5)
#define ST25R3911_ST25R3911_REG_NFCIP1_BIT_RATE_nfc_rate2     (1U<<6)
#define ST25R3911_ST25R3911_REG_NFCIP1_BIT_RATE_nfc_rate3     (1U<<7)
#define ST25R3911_REG_NFCIP1_BIT_RATE_nfc_rate_mask           (0xfU<<4)
#define ST25R3911_REG_NFCIP1_BIT_RATE_nfc_rate_shift          (4U)
#define ST25R3911_REG_ANT_CAL_CONTROL_mask_tre                (0xfU<<3)
#define ST25R3911_REG_ANT_CAL_CONTROL_shift_tre               (3U)
#define ST25R3911_REG_ANT_CAL_CONTROL_tre_0                   (1U<<3)
#define ST25R3911_REG_ANT_CAL_CONTROL_tre_1                   (1U<<4)
#define ST25R3911_REG_ANT_CAL_CONTROL_tre_2                   (1U<<5)
#define ST25R3911_REG_ANT_CAL_CONTROL_tre_3                   (1U<<6)
#define ST25R3911_REG_ANT_CAL_CONTROL_trim_s                  (1U<<7)
#define ST25R3911_REG_ANT_CAL_RESULT_tri_err                  (1U<<3)
#define ST25R3911_REG_ANT_CAL_RESULT_tri_0                    (1U<<4)
#define ST25R3911_REG_ANT_CAL_RESULT_tri_1                    (1U<<5)
#define ST25R3911_REG_ANT_CAL_RESULT_tri_2                    (1U<<6)
#define ST25R3911_REG_ANT_CAL_RESULT_tri_3                    (1U<<7)
#define ST25R3911_REG_AM_MOD_DEPTH_CONTROL_mask_mod           (0x3fU<<1)
#define ST25R3911_REG_AM_MOD_DEPTH_CONTROL_shift_mod          (1U)
#define ST25R3911_REG_AM_MOD_DEPTH_CONTROL_mod_8percent       (0xbU<<1)
#define ST25R3911_REG_AM_MOD_DEPTH_CONTROL_mod_10percent      (0xeU<<1)
#define ST25R3911_REG_AM_MOD_DEPTH_CONTROL_mod_14percent      (0x14U<<1)
#define ST25R3911_REG_AM_MOD_DEPTH_CONTROL_mod_20percent      (0x20U<<1)
#define ST25R3911_REG_AM_MOD_DEPTH_CONTROL_mod_25percent      (0x2aU<<1)
#define ST25R3911_REG_AM_MOD_DEPTH_CONTROL_mod_30percent      (0x37U<<1)
#define ST25R3911_REG_AM_MOD_DEPTH_CONTROL_mod_33percent      (0x3fU<<1)
#define ST25R3911_REG_AM_MOD_DEPTH_CONTROL_am_s               (1U<<7)
#define ST25R3911_REG_RFO_AM_MOD_LEVEL_dram0                  (1U<<0)
#define ST25R3911_REG_RFO_AM_MOD_LEVEL_dram1                  (1U<<1)
#define ST25R3911_REG_RFO_AM_MOD_LEVEL_dram2                  (1U<<2)
#define ST25R3911_REG_RFO_AM_MOD_LEVEL_dram3                  (1U<<3)
#define ST25R3911_REG_RFO_AM_MOD_LEVEL_dram4                  (1U<<4)
#define ST25R3911_REG_RFO_AM_MOD_LEVEL_dram5                  (1U<<5)
#define ST25R3911_REG_RFO_AM_MOD_LEVEL_dram6                  (1U<<6)
#define ST25R3911_REG_RFO_AM_MOD_LEVEL_dram7                  (1U<<7)
#define ST25R3911_REG_FIELD_THRESHOLD_rfe_t0                  (1U<<0)
#define ST25R3911_REG_FIELD_THRESHOLD_rfe_t1                  (1U<<1)
#define ST25R3911_REG_FIELD_THRESHOLD_rfe_t2                  (1U<<2)
#define ST25R3911_REG_FIELD_THRESHOLD_rfe_t3                  (1U<<3)
#define ST25R3911_REG_FIELD_THRESHOLD_trg_l0                  (1U<<4)
#define ST25R3911_REG_FIELD_THRESHOLD_trg_l1                  (1U<<5)
#define ST25R3911_REG_FIELD_THRESHOLD_trg_l2                  (1U<<6)
#define ST25R3911_REG_FIELD_THRESHOLD_mask_trg                (0x07U<<4)
#define ST25R3911_REG_FIELD_THRESHOLD_trg_75mV                (0x00U<<4)
#define ST25R3911_REG_FIELD_THRESHOLD_trg_105mV               (0x01U<<4)
#define ST25R3911_REG_FIELD_THRESHOLD_trg_150mV               (0x02U<<4)
#define ST25R3911_REG_FIELD_THRESHOLD_trg_205mV               (0x03U<<4)
#define ST25R3911_REG_FIELD_THRESHOLD_trg_290mV               (0x04U<<4)
#define ST25R3911_REG_FIELD_THRESHOLD_trg_400mV               (0x05U<<4)
#define ST25R3911_REG_FIELD_THRESHOLD_trg_560mV               (0x06U<<4)
#define ST25R3911_REG_FIELD_THRESHOLD_trg_800mV               (0x07U<<4)
#define ST25R3911_REG_FIELD_THRESHOLD_mask_rfe                (0x0FU<<0)
#define ST25R3911_REG_FIELD_THRESHOLD_rfe_75mV                (0x00U<<0)
#define ST25R3911_REG_FIELD_THRESHOLD_rfe_105mV               (0x01U<<0)
#define ST25R3911_REG_FIELD_THRESHOLD_rfe_150mV               (0x02U<<0)
#define ST25R3911_REG_FIELD_THRESHOLD_rfe_205mV               (0x03U<<0)
#define ST25R3911_REG_FIELD_THRESHOLD_rfe_290mV               (0x04U<<0)
#define ST25R3911_REG_FIELD_THRESHOLD_rfe_400mV               (0x05U<<0)
#define ST25R3911_REG_FIELD_THRESHOLD_rfe_560mV               (0x06U<<0)
#define ST25R3911_REG_FIELD_THRESHOLD_rfe_800mV               (0x07U<<0)
#define ST25R3911_REG_FIELD_THRESHOLD_rfe_25mV                (0x08U<<0)
#define ST25R3911_REG_FIELD_THRESHOLD_rfe_33mV                (0x09U<<0)
#define ST25R3911_REG_FIELD_THRESHOLD_rfe_47mV                (0x0AU<<0)
#define ST25R3911_REG_FIELD_THRESHOLD_rfe_64mV                (0x0BU<<0)
#define ST25R3911_REG_FIELD_THRESHOLD_rfe_90mV                (0x0CU<<0)
#define ST25R3911_REG_FIELD_THRESHOLD_rfe_125mV               (0x0DU<<0)
#define ST25R3911_REG_FIELD_THRESHOLD_rfe_175mV               (0x0EU<<0)
#define ST25R3911_REG_FIELD_THRESHOLD_rfe_250mV               (0x0FU<<0)
#define ST25R3911_REG_REGULATOR_CONTROL_shift_mpsv            (1U)
#define ST25R3911_REG_REGULATOR_CONTROL_mask_mpsv             (3U<<1)
#define ST25R3911_REG_REGULATOR_CONTROL_mpsv_vdd              (0U<<1)
#define ST25R3911_REG_REGULATOR_CONTROL_mpsv_vsp_a            (1U<<1)
#define ST25R3911_REG_REGULATOR_CONTROL_mpsv_vsp_d            (2U<<1)
#define ST25R3911_REG_REGULATOR_CONTROL_mpsv_vsp_rf           (3U<<1)
#define ST25R3911_REG_REGULATOR_CONTROL_mask_rege             (0xfU<<3)
#define ST25R3911_REG_REGULATOR_CONTROL_shift_rege            (3U)
#define ST25R3911_REG_REGULATOR_CONTROL_reg_s                 (1U<<7)
#define ST25R3911_REG_REGULATOR_RESULT_mrt_on                 (1U<<0)
#define ST25R3911_REG_REGULATOR_RESULT_nrt_on                 (1U<<1)
#define ST25R3911_REG_REGULATOR_RESULT_gpt_on                 (1U<<2)
#define ST25R3911_REG_REGULATOR_RESULT_mask_reg               (0xfU<<4)
#define ST25R3911_REG_REGULATOR_RESULT_shift_reg              (4U)
#define ST25R3911_REG_REGULATOR_RESULT_reg_0                  (1U<<4)
#define ST25R3911_REG_REGULATOR_RESULT_reg_1                  (1U<<5)
#define ST25R3911_REG_REGULATOR_RESULT_reg_2                  (1U<<6)
#define ST25R3911_REG_REGULATOR_RESULT_reg_3                  (1U<<7)
#define ST25R3911_REG_RSSI_RESULT_mask_rssi_pm                (0xfU)
#define ST25R3911_REG_RSSI_RESULT_shift_rssi_pm               (0U)
#define ST25R3911_REG_RSSI_RESULT_rssi_pm0                    (1U<<0)
#define ST25R3911_REG_RSSI_RESULT_rssi_pm1                    (1U<<1)
#define ST25R3911_REG_RSSI_RESULT_rssi_pm2                    (1U<<2)
#define ST25R3911_REG_RSSI_RESULT_rssi_pm3                    (1U<<3)
#define ST25R3911_REG_RSSI_RESULT_mask_rssi_am                (0xfU<<4)
#define ST25R3911_REG_RSSI_RESULT_shift_rssi_am               (4U)
#define ST25R3911_REG_RSSI_RESULT_rssi_am_0                   (1U<<4)
#define ST25R3911_REG_RSSI_RESULT_rssi_am_1                   (1U<<5)
#define ST25R3911_REG_RSSI_RESULT_rssi_am_2                   (1U<<6)
#define ST25R3911_REG_RSSI_RESULT_rssi_am_3                   (1U<<7)
#define ST25R3911_REG_GAIN_RED_STATE_mask_gs_pm               (0xfU)
#define ST25R3911_REG_GAIN_RED_STATE_shift_gs_pm              (0U)
#define ST25R3911_REG_GAIN_RED_STATE_gs_pm_0                  (1U<<0)
#define ST25R3911_REG_GAIN_RED_STATE_gs_pm_1                  (1U<<1)
#define ST25R3911_REG_GAIN_RED_STATE_gs_pm_2                  (1U<<2)
#define ST25R3911_REG_GAIN_RED_STATE_gs_pm_3                  (1U<<3)
#define ST25R3911_REG_GAIN_RED_STATE_mask_gs_am               (0xfU<<4)
#define ST25R3911_REG_GAIN_RED_STATE_shift_gs_am              (4U)
#define ST25R3911_REG_GAIN_RED_STATE_gs_am_0                  (1U<<4)
#define ST25R3911_REG_GAIN_RED_STATE_gs_am_1                  (1U<<5)
#define ST25R3911_REG_GAIN_RED_STATE_gs_am_2                  (1U<<6)
#define ST25R3911_REG_GAIN_RED_STATE_gs_am_3                  (1U<<7)
#define ST25R3911_REG_CAP_SENSOR_CONTROL_cs_g0                (1U<<0)
#define ST25R3911_REG_CAP_SENSOR_CONTROL_cs_g1                (1U<<1)
#define ST25R3911_REG_CAP_SENSOR_CONTROL_cs_g2                (1U<<2)
#define ST25R3911_REG_CAP_SENSOR_CONTROL_mask_cs_g            (7U<<0)
#define ST25R3911_REG_CAP_SENSOR_CONTROL_cs_mcal0             (1U<<3)
#define ST25R3911_REG_CAP_SENSOR_CONTROL_cs_mcal1             (1U<<4)
#define ST25R3911_REG_CAP_SENSOR_CONTROL_cs_mcal2             (1U<<5)
#define ST25R3911_REG_CAP_SENSOR_CONTROL_cs_mcal3             (1U<<6)
#define ST25R3911_REG_CAP_SENSOR_CONTROL_cs_mcal4             (1U<<7)
#define ST25R3911_REG_CAP_SENSOR_CONTROL_mask_cs_mcal         (0x1fU<<3)
#define ST25R3911_REG_CAP_SENSOR_CONTROL_shift_cs_mcal        (3U)
#define ST25R3911_REG_CAP_SENSOR_RESULT_cs_cal_err            (1U<<1)
#define ST25R3911_REG_CAP_SENSOR_RESULT_cs_cal_end            (1U<<2)
#define ST25R3911_REG_CAP_SENSOR_RESULT_cs_cal0               (1U<<3)
#define ST25R3911_REG_CAP_SENSOR_RESULT_cs_cal1               (1U<<4)
#define ST25R3911_REG_CAP_SENSOR_RESULT_cs_cal2               (1U<<5)
#define ST25R3911_REG_CAP_SENSOR_RESULT_cs_cal3               (1U<<6)
#define ST25R3911_REG_CAP_SENSOR_RESULT_cs_cal4               (1U<<7)
#define ST25R3911_REG_AUX_DISPLAY_en_ac                       (1U<<0)
#define ST25R3911_REG_AUX_DISPLAY_nfc_t                       (1U<<1)
#define ST25R3911_REG_AUX_DISPLAY_rx_act                      (1U<<2)
#define ST25R3911_REG_AUX_DISPLAY_rx_on                       (1U<<3)
#define ST25R3911_REG_AUX_DISPLAY_osc_ok                      (1U<<4)
#define ST25R3911_REG_AUX_DISPLAY_tx_on                       (1U<<5)
#define ST25R3911_REG_AUX_DISPLAY_efd_o                       (1U<<6)
#define ST25R3911_REG_AUX_DISPLAY_a_cha                       (1U<<7)
#define ST25R3911_REG_WUP_TIMER_CONTROL_wcap                  (1U<<0)
#define ST25R3911_REG_WUP_TIMER_CONTROL_wph                   (1U<<1)
#define ST25R3911_REG_WUP_TIMER_CONTROL_wam                   (1U<<2)
#define ST25R3911_REG_WUP_TIMER_CONTROL_wto                   (1U<<3)
#define ST25R3911_REG_WUP_TIMER_CONTROL_wut0                  (1U<<4)
#define ST25R3911_REG_WUP_TIMER_CONTROL_wut1                  (1U<<5)
#define ST25R3911_REG_WUP_TIMER_CONTROL_wut2                  (1U<<6)
#define ST25R3911_REG_WUP_TIMER_CONTROL_shift_wut             (4U)
#define ST25R3911_REG_WUP_TIMER_CONTROL_mask_wut              (7U<<4)
#define ST25R3911_REG_WUP_TIMER_CONTROL_wur                   (1U<<7)
#define ST25R3911_REG_AMPLITUDE_MEASURE_CONF_am_ae            (1U<<0)
#define ST25R3911_REG_AMPLITUDE_MEASURE_CONF_am_aew0          (1U<<1)
#define ST25R3911_REG_AMPLITUDE_MEASURE_CONF_am_aew1          (1U<<2)
#define ST25R3911_REG_AMPLITUDE_MEASURE_CONF_shift_am_aew     (1U)
#define ST25R3911_REG_AMPLITUDE_MEASURE_CONF_mask_am_aew      (3U<<1)
#define ST25R3911_REG_AMPLITUDE_MEASURE_CONF_am_aam           (1U<<3)
#define ST25R3911_REG_AMPLITUDE_MEASURE_CONF_am_d0            (1U<<4)
#define ST25R3911_REG_AMPLITUDE_MEASURE_CONF_am_d1            (1U<<5)
#define ST25R3911_REG_AMPLITUDE_MEASURE_CONF_am_d2            (1U<<6)
#define ST25R3911_REG_AMPLITUDE_MEASURE_CONF_am_d3            (1U<<7)
#define ST25R3911_REG_AMPLITUDE_MEASURE_CONF_shift_am_d       (4U)
#define ST25R3911_REG_AMPLITUDE_MEASURE_CONF_mask_am_d        (0xfU<<4)
#define ST25R3911_REG_PHASE_MEASURE_CONF_pm_ae                (1U<<0)
#define ST25R3911_REG_PHASE_MEASURE_CONF_pm_aew0              (1U<<1)
#define ST25R3911_REG_PHASE_MEASURE_CONF_pm_aew1              (1U<<2)
#define ST25R3911_REG_PHASE_MEASURE_CONF_shift_pm_aew         (1U)
#define ST25R3911_REG_PHASE_MEASURE_CONF_mask_pm_aew          (3U<<1)
#define ST25R3911_REG_PHASE_MEASURE_CONF_pm_aam               (1U<<3)
#define ST25R3911_REG_PHASE_MEASURE_CONF_pm_d0                (1U<<4)
#define ST25R3911_REG_PHASE_MEASURE_CONF_pm_d1                (1U<<5)
#define ST25R3911_REG_PHASE_MEASURE_CONF_pm_d2                (1U<<6)
#define ST25R3911_REG_PHASE_MEASURE_CONF_pm_d3                (1U<<7)
#define ST25R3911_REG_PHASE_MEASURE_CONF_shift_pm_d           (4U)
#define ST25R3911_REG_PHASE_MEASURE_CONF_mask_pm_d            (0xfU<<4)
#define ST25R3911_REG_CAPACITANCE_MEASURE_CONF_cm_ae          (1U<<0)
#define ST25R3911_REG_CAPACITANCE_MEASURE_CONF_cm_aew0        (1U<<1)
#define ST25R3911_REG_CAPACITANCE_MEASURE_CONF_cm_aew1        (1U<<2)
#define ST25R3911_REG_CAPACITANCE_MEASURE_CONF_shift_cm_aew   (1U)
#define ST25R3911_REG_CAPACITANCE_MEASURE_CONF_mask_cm_aew    (3U<<1)
#define ST25R3911_REG_CAPACITANCE_MEASURE_CONF_cm_aam         (1U<<3)
#define ST25R3911_REG_CAPACITANCE_MEASURE_CONF_cm_d0          (1U<<4)
#define ST25R3911_REG_CAPACITANCE_MEASURE_CONF_cm_d1          (1U<<5)
#define ST25R3911_REG_CAPACITANCE_MEASURE_CONF_cm_d2          (1U<<6)
#define ST25R3911_REG_CAPACITANCE_MEASURE_CONF_cm_d3          (1U<<7)
#define ST25R3911_REG_CAPACITANCE_MEASURE_CONF_shift_cm_d     (4U)
#define ST25R3911_REG_IC_IDENTITY_v2                          (0x09U)
#define ST25R3911_REG_IC_IDENTITY_ic_type                     (1U<<3)
#define ST25R3911_REG_IC_IDENTITY_mask_ic_type                (0x1FU<<3)
#define ST25R3911_REG_IC_IDENTITY_shift_ic_type               (3U)
#define ST25R3911_REG_IC_IDENTITY_mask_ic_rev                 (7U)

/*! \endcond DOXYGEN_SUPPRESS */

/*
******************************************************************************
* GLOBAL FUNCTION PROTOTYPES
******************************************************************************
*/
/*! 
 *****************************************************************************
 *  \brief  Returns the content of a register within the ST25R3911
 *
 *  This function is used to read out the content of ST25R3911 registers.
 *
 *  \param[in]  reg: Address of register to read.
 *  \param[out] value: Returned value.
 *
 *****************************************************************************
 */
extern void st25r3911ReadRegister(uint8_t reg, uint8_t* value);

/*! 
 *****************************************************************************
 *  \brief  Writes a given value to a register within the ST25R3911
 *
 *  This function is used to write \a value to address \a reg within the ST25R3911.
 *
 *  \param[in]  reg: Address of the register to write.
 *  \param[in]  value: Value to be written.
 *
 *****************************************************************************
 */
extern void st25r3911WriteRegister(uint8_t reg, uint8_t value);

/*! 
 *****************************************************************************
 *  \brief  Cleart bits on Register
 *
 *  This function clears the given bitmask on the register 
 *
 *  \warning This method does not guarantee consistency of register content 
 *           when called from multiple contexts (task, ISR, thread)
 *
 *  \param[in]  reg: Address of the register clear
 *  \param[in]  clr_mask: Bitmask of bit to be cleared
 *
 *****************************************************************************
 */
extern void st25r3911ClrRegisterBits( uint8_t reg, uint8_t clr_mask );


/*! 
 *****************************************************************************
 *  \brief  Set bits on Register
 *
 *  This function sets the given bitmask on the register 
 *
 *  \warning This method does not guarantee consistency of register content 
 *           when called from multiple contexts (task, ISR, thread)
 *
 *  \param[in]  reg: Address of the register clear
 *  \param[in]  set_mask: Bitmask of bit to be cleared
 *
 *****************************************************************************
 */
extern void st25r3911SetRegisterBits( uint8_t reg, uint8_t set_mask );


/*! 
 *****************************************************************************
 *  \brief  Changes the given bits on a ST25R3911 register
 *
 *  This function is used if only a particular bits should be changed within
 *  an ST25R3911 register.
 *
 *  \warning This method does not guarantee consistency of register content 
 *           when called from multiple contexts (task, ISR, thread)
 *
 *  \param[in]  reg: Address of the register to write.
 *  \param[in]  valueMask: bitmask of bits to be changed
 *  \param[in]  value: the bits to be written on the enabled valueMask bits
 *
 *****************************************************************************
 */
extern void st25r3911ChangeRegisterBits(uint8_t reg, uint8_t valueMask, uint8_t value);

/*! 
 *****************************************************************************
 *  \brief  Read a test register within the ST25R3911
 *
 *  This function is used to read the content of test address \a reg within the ST25R3911.
 *
 *  \param[in]   reg: Address of the register to read.
 *  \param[out]  value: Returned read.
 *
 *****************************************************************************
 */
extern void st25r3911ReadTestRegister(uint8_t reg, uint8_t* value);

/*! 
 *****************************************************************************
 *  \brief  Writes a given value to a test register within the ST25R3911
 *
 *  This function is used to write \a value to test address \a reg within the ST25R3911.
 *
 *  \param[in]  reg: Address of the register to write.
 *  \param[in]  value: Value to be written.
 *
 *****************************************************************************
 */
extern void st25r3911WriteTestRegister(uint8_t reg, uint8_t value);

/*! 
 *****************************************************************************
 *  \brief  Modifies a value within a ST25R3911 register
 *
 *  This function is used if only a particular bits should be changed within
 *  an ST25R3911 register.
 *
 *  \warning This method does not guarantee consistency of register content 
 *           when called from multiple contexts (task, ISR, thread)
 * 
 *  \param[in]  reg: Address of the register to write.
 *  \param[in]  clr_mask: bitmask of bits to be cleared to 0.
 *  \param[in]  set_mask: bitmask of bits to be set to 1.
 *
 *****************************************************************************
 */
extern void st25r3911ModifyRegister(uint8_t reg, uint8_t clr_mask, uint8_t set_mask);


/*! 
 *****************************************************************************
 *  \brief  Changes the given bits on a ST25R3911 Test register
 *
 *  This function is used if only a particular bits should be changed within
 *  an ST25R3911 register.
 *
 *  \param[in]  reg: Address of the Test register to change.
 *  \param[in]  valueMask: bitmask of bits to be changed
 *  \param[in]  value: the bits to be written on the enabled valueMask bits
 *
 *  \warning This method does not guarantee consistency of register content 
 *           when called from multiple contexts (task, ISR, thread)
 *****************************************************************************
 */
extern void st25r3911ChangeTestRegisterBits( uint8_t reg, uint8_t valueMask, uint8_t value );

/*! 
 *****************************************************************************
 *  \brief  Writes multiple values to ST25R3911 registers
 *
 *  This function is used to write multiple values to the ST25R3911 using the
 *  auto-increment feature. That is, after each write the address pointer
 *  inside the ST25R3911 gets incremented automatically.
 *
 *  \param[in]  reg: Address of the frist register to write.
 *  \param[in]  values: pointer to a buffer containing the values to be written.
 *  \param[in]  length: Number of values to be written.
 *
 *****************************************************************************
 */
extern void st25r3911WriteMultipleRegisters(uint8_t reg, const uint8_t* values, uint8_t length);

/*! 
 *****************************************************************************
 *  \brief  Reads from multiple ST25R3911 registers
 *
 *  This function is used to read from multiple registers using the
 *  auto-increment feature. That is, after each read the address pointer
 *  inside the ST25R3911 gets incremented automatically.
 *
 *  \param[in]  reg: Address of the frist register to read from.
 *  \param[in]  values: pointer to a buffer where the result shall be written to.
 *  \param[in]  length: Number of registers to be read out.
 *
 *****************************************************************************
 */
extern void st25r3911ReadMultipleRegisters(uint8_t reg, uint8_t* values, uint8_t length);

/*! 
 *****************************************************************************
 *  \brief  Writes values to ST25R3911 FIFO
 *
 *  This function needs to be called in order to write to the ST25R3911 FIFO.
 *
 *  \param[in]  values: pointer to a buffer containing the values to be written
 *                      to the FIFO.
 *  \param[in]  length: Number of values to be written.
 *
 *****************************************************************************
 */
extern void st25r3911WriteFifo(const uint8_t* values, uint8_t length);

/*! 
 *****************************************************************************
 *  \brief  Read values from ST25R3911 FIFO
 *
 *  This function needs to be called in order to read from ST25R3911 FIFO.
 *
 *  \param[out]  buf: pointer to a buffer where the FIFO content shall be
 *                       written to.
 *  \param[in]  length: Number of bytes to read. (= size of \a buf)
 *  \note: This function doesn't check whether \a length is really the
 *  number of available bytes in FIFO
 *
 *****************************************************************************
 */
extern void st25r3911ReadFifo(uint8_t* buf, uint8_t length);

/*! 
 *****************************************************************************
 *  \brief  Execute a direct command
 *
 *  This function is used to start so-called direct command. These commands
 *  are implemented inside the chip and each command has unique code (see
 *  datasheet).
 *
 *  \param[in]  cmd : code of the direct command to be executed.
 *
 *****************************************************************************
 */
extern void st25r3911ExecuteCommand(uint8_t cmd);

/*! 
 *****************************************************************************
 *  \brief  Execute several direct commands
 *
 *  This function is used to start so-called direct command. These commands
 *  are implemented inside the chip and each command has unique code (see
 *  datasheet).
 *
 *  \param[in]  cmds   : codes of the direct command to be executed.
 *  \param[in]  length : number of commands to be executed 
 *
 *****************************************************************************
 */
extern void st25r3911ExecuteCommands(const uint8_t *cmds, uint8_t length);

/*! 
 *****************************************************************************
 *  \brief  Check if register ID is valid
 *
 *  Checks if the given register ID a valid ST25R3911 register
 *
 *  \param[in]  reg: Address of register to check
 *  
 *  \return  true if is a valid register ID
 *  \return  false otherwise
 *
 *****************************************************************************
 */
extern bool st25r3911IsRegValid( uint8_t reg );

#endif /* ST25R3911_COM_H */

/**
  * @}
  *
  * @}
  *
  * @}
  * 
  * @}
  */

