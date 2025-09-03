// clang-format off

/******************************************************************************
  *
  * @attention
  *
  * COPYRIGHT 2018 STMicroelectronics, all rights reserved
  * COPYRIGHT (c) 2025 Nordic Semiconductor ASA
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
/*! \file
 *
 *  \author
 *
 *  \brief Platform header file. Defining platform independent functionality.
 *
 */

#ifndef RFAL_PLATFORM_H
#define RFAL_PLATFORM_H

#ifdef __cplusplus
extern "C" {
#endif

/*
******************************************************************************
* INCLUDES
******************************************************************************
*/

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>
#include <math.h>

#include "ncs_pal_spi.h"
#include "ncs_pal_timer.h"
#include "ncs_pal_gpio.h"
#include "ncs_pal_isr.h"
#include "ncs_pal_critical_section.h"

#include "rfal_platform_config.h"

/*
******************************************************************************
* GLOBAL DEFINES
******************************************************************************
*/

#define ST25R_COM_SINGLETXRX                              /*!< Static buffer for SPI communication */

#define ST25R_SS_PIN             0                        /*!< GPIO pin used for ST25R SPI SS                */
#define ST25R_SS_PORT            0                        /*!< GPIO port used for ST25R SPI SS port          */

#define ST25R_INT_PIN            0                        /*!< GPIO pin used for ST25R External Interrupt    */
#define ST25R_INT_PORT           0                        /*!< GPIO port used for ST25R External Interrupt   */

#define ST25R_RESET_PIN          0                        /*!< GPIO pin used for ST25R Reset                 */
#define ST25R_RESET_PORT         0                        /*!< GPIO port used for ST25R Reset                */


#ifdef LED_FIELD_Pin
#define PLATFORM_LED_FIELD_PIN   LED_FIELD_Pin            /*!< LED 1 GPIO pin used as field LED              */
#endif

#ifdef LED_FIELD_GPIO_Port
#define PLATFORM_LED_FIELD_PORT  LED_FIELD_GPIO_Port      /*!< LED 1 GPIO port used as field LED             */
#endif

/*
******************************************************************************
* GLOBAL MACROS
******************************************************************************
*/
#define platformProtectST25RComm()                    ncs_pal_critical_section_start()                        /*!< Protect unique access to ST25R communication channel - IRQ disable on single thread environment (MCU) ; Mutex lock on a multi thread environment      */
#define platformUnprotectST25RComm()                  ncs_pal_critical_section_stop()                      /*!< Unprotect unique access to ST25R communication channel - IRQ enable on a single thread environment (MCU) ; Mutex unlock on a multi thread environment */

#define platformProtectST25RIrqStatus()               platformProtectST25RComm()                    /*!< Protect unique access to IRQ status var - IRQ disable on single thread environment (MCU) ; Mutex lock on a multi thread environment */
#define platformUnprotectST25RIrqStatus()             platformUnprotectST25RComm()                  /*!< Unprotect the IRQ status var - IRQ enable on a single thread environment (MCU) ; Mutex unlock on a multi thread environment         */

#define platformLedOff( port, pin )                                                                 /*!< Turns the given LED Off                     */
#define platformLedOn( port, pin )                                                                  /*!< Turns the given LED On                      */
#define platformLedToggle( port, pin )                                                              /*!< Toggles the given LED                       */

#define platformGpioSet( port, pin )                                                                /*!< Turns the given GPIO High                   */
#define platformGpioClear( port, pin )                                                              /*!< Turns the given GPIO Low                    */
#define platformGpioToggle( port, pin )                                                             /*!< Toggles the given GPIO                      */
#define platformGpioIsHigh( port, pin )               ncs_pal_gpio_is_set(port,pin)                 /*!< Checks if the given LED is High             */
#define platformGpioIsLow( port, pin )                (!platformGpioIsHigh(port, pin))              /*!< Checks if the given LED is Low              */

#define platformTimerCreate( t )                      ncs_pal_timer_create(t)                       /*!< Create a timer with the given time (ms)     */
#define platformTimerIsExpired( timer )               ncs_pal_timer_is_expired(timer)               /*!< Checks if the given timer is expired        */
#define platformTimerDestroy( timer )                 ncs_pal_timer_destroy(timer)                  /*!< Stop and release the given timer            */
#define platformDelay( t )                            ncs_pal_delay(t)                              /*!< Performs a delay for the given time (ms)    */
#define platformTimerGetRemaining( timer )            ncs_pal_timer_get_remaining(timer)            /*!< Get the remaining time of the given timer   */

#define platformGetSysTick()                          ncs_pal_get_sys_tick()                        /*!< Get System Tick ( 1 tick = 1 ms)            */

#define platformAssert( exp )                                                                       /*!< Asserts whether the given expression is true*/
#define platformErrorHandle()                                                                       /*!< Global error handle\trap                    */


#define platformSpiSelect()                                                                         /*!< SPI SS\CS: Chip|Slave Select                */
#define platformSpiDeselect()                                                                       /*!< SPI SS\CS: Chip|Slave Deselect              */
#define platformSpiTxRx( txBuf, rxBuf, len )          ncs_pal_spi_transfer(txBuf, rxBuf, len)       /*!< SPI transceive                              */


/*
******************************************************************************
* GLOBAL VARIABLES
******************************************************************************
*/
extern uint8_t globalCommProtectCnt;                      /* Global Protection Counter provided per platform - instantiated in main.c    */

/*
******************************************************************************
* RFAL CUSTOM SETTINGS
******************************************************************************
*/

#ifndef platformProtectST25RIrqStatus
    #define platformProtectST25RIrqStatus()            /*!< Protect unique access to IRQ status var - IRQ disable on single thread environment (MCU) ; Mutex lock on a multi thread environment */
#endif /* platformProtectST25RIrqStatus */

#ifndef platformUnprotectST25RIrqStatus
    #define platformUnprotectST25RIrqStatus()          /*!< Unprotect the IRQ status var - IRQ enable on a single thread environment (MCU) ; Mutex unlock on a multi thread environment         */
#endif /* platformUnprotectST25RIrqStatus */

#ifndef platformProtectWorker
    #define platformProtectWorker()                    /* Protect RFAL Worker/Task/Process from concurrent execution on multi thread platforms   */
#endif /* platformProtectWorker */

#ifndef platformUnprotectWorker
    #define platformUnprotectWorker()                  /* Unprotect RFAL Worker/Task/Process from concurrent execution on multi thread platforms */
#endif /* platformUnprotectWorker */

#ifndef platformIrqST25RPinInitialize
    #define platformIrqST25RPinInitialize()            ncs_pal_gpio_init() /*!< Initializes ST25R IRQ pin                     */
#endif /* platformIrqST25RPinInitialize */

#ifndef platformIrqST25RSetCallback
    #define platformIrqST25RSetCallback( cb )          ncs_pal_isr_cb_set(cb) /*!< Sets ST25R ISR callback                       */
#endif /* platformIrqST25RSetCallback */

#ifndef platformLedsInitialize
    #define platformLedsInitialize()                   /*!< Initializes the pins used as LEDs to outputs  */
#endif /* platformLedsInitialize */

#ifndef platformLedOff
    #define platformLedOff( port, pin )                /*!< Turns the given LED Off                       */
#endif /* platformLedOff */

#ifndef platformLedOn
    #define platformLedOn( port, pin )                 /*!< Turns the given LED On                        */
#endif /* platformLedOn */

#ifndef platformLedToggle
    #define platformLedToggle( port, pin )             /*!< Toggles the given LED                         */
#endif /* platformLedToggle */

#ifndef platformGetSysTick
    #define platformGetSysTick()                       /*!< Get System Tick ( 1 tick = 1 ms)              */
#endif /* platformGetSysTick */

#ifndef platformTimerDestroy
    #define platformTimerDestroy( timer )              /*!< Stops and released the given timer            */
#endif /* platformTimerDestroy */

#ifndef platformLog
    #define platformLog(...)                           /*!< Log method                                    */
#endif /* platformLog */

#ifndef platformAssert
    #define platformAssert( exp )                      /*!< Asserts whether the given expression is true */
#endif /* platformAssert */

#ifndef platformErrorHandle
    #define platformErrorHandle()                      /*!< Global error handler or trap                 */
#endif /* platformErrorHandle */

#ifdef __cplusplus
}
#endif

#endif /* RFAL_PLATFORM_H */
