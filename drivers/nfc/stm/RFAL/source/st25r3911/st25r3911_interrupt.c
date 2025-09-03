
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
 *  \brief ST25R3911 Interrupt handling
 *
 */

/*
******************************************************************************
* INCLUDES
******************************************************************************
*/
#include "st25r3911_interrupt.h"
#include "st25r3911_com.h"
#include "st25r3911.h"
#include "rfal_utils.h"

/*
******************************************************************************
* GLOBAL DEFINES
******************************************************************************
*/

/*! Length of the interrupt registers       */
#define ST25R3911_INT_REGS_LEN          ( (ST25R3911_REG_IRQ_ERROR_WUP - ST25R3911_REG_IRQ_MAIN) + 1U )

/*
 ******************************************************************************
 * LOCAL DATA TYPES
 ******************************************************************************
 */

/*! Holds current and previous interrupt callback pointer as well as current Interrupt status and mask */
typedef struct
{
    void      (*prevCallback)(void); /*!< call back function for 3911 interrupt               */
    void      (*callback)(void);     /*!< call back function for 3911 interrupt               */
    uint32_t  status;                /*!< latest interrupt status                             */
    uint32_t  mask;                  /*!< Interrupt mask. Negative mask = ST25R3911 mask regs */
    bool      hasNRE;                /*!< Last IRQ had NRE flag                               */
}t_st25r3911Interrupt;

/*
******************************************************************************
* GLOBAL VARIABLES
******************************************************************************
*/

static volatile t_st25r3911Interrupt st25r3911interrupt; /*!< Instance of ST25R3911 interrupt */


static void st25r3911IRQCheck( uint32_t irqStatus );

/*
******************************************************************************
* GLOBAL FUNCTIONS
******************************************************************************
*/
void st25r3911InitInterrupts( void )
{
    platformIrqST25RPinInitialize();
    platformIrqST25RSetCallback( st25r3911Isr );
    
    st25r3911interrupt.callback     = NULL;
    st25r3911interrupt.prevCallback = NULL;
    st25r3911interrupt.status       = ST25R3911_IRQ_MASK_NONE;
    st25r3911interrupt.mask         = ST25R3911_IRQ_MASK_NONE;
    
    /* Initialize LEDs if existing and defined */
    platformLedsInitialize();

#ifdef PLATFORM_LED_RX_PIN
    platformLedOff( PLATFORM_LED_RX_PORT, PLATFORM_LED_RX_PIN );
#endif /* PLATFORM_LED_RX_PIN */

#ifdef PLATFORM_LED_FIELD_PIN
    platformLedOff( PLATFORM_LED_FIELD_PORT, PLATFORM_LED_FIELD_PIN );
#endif /* PLATFORM_LED_FIELD_PIN */
    
#ifdef PLATFORM_LED_ERR_PIN
    platformLedOff( PLATFORM_LED_ERR_PORT, PLATFORM_LED_ERR_PIN );
#endif /* PLATFORM_LED_ERR_PIN */
    
}

void st25r3911Isr( void )
{
    st25r3911CheckForReceivedInterrupts();
    
    if (NULL != st25r3911interrupt.callback)
    {
        st25r3911interrupt.callback();
    }
}

void st25r3911CheckForReceivedInterrupts( void )
{
    uint8_t  iregs[ST25R3911_INT_REGS_LEN];
    uint32_t irqStatus; 

    irqStatus = ST25R3911_IRQ_MASK_NONE;
    RFAL_MEMSET( iregs, (int32_t)(ST25R3911_IRQ_MASK_ALL & 0xFFU), ST25R3911_INT_REGS_LEN );  /* MISRA 10.3 */
        
    /* In case the IRQ is Edge (not Level) triggered read IRQs until done */
    while( platformGpioIsHigh( ST25R_INT_PORT, ST25R_INT_PIN ) )
    {
        st25r3911ReadMultipleRegisters(ST25R3911_REG_IRQ_MAIN, iregs, sizeof(iregs));
       
        if ((iregs[0] & ST25R3911_IRQ_MASK_TXE) != 0U)
        {
        #ifdef PLATFORM_LED_FIELD_PIN         
            platformLedOn( PLATFORM_LED_FIELD_PORT, PLATFORM_LED_FIELD_PIN );
        #endif /* PLATFORM_LED_FIELD_PIN */
        
        #ifdef PLATFORM_LED_ERR_PIN         
            platformLedOff( PLATFORM_LED_ERR_PORT, PLATFORM_LED_ERR_PIN );
        #endif /* PLATFORM_LED_ERR_PIN */
        }
       
    #ifdef PLATFORM_LED_RX_PIN
        if ((iregs[0] & ST25R3911_IRQ_MASK_RXS) != 0U)
        {
            platformLedOn( PLATFORM_LED_RX_PORT, PLATFORM_LED_RX_PIN );
        }
        if (((iregs[0] & ST25R3911_IRQ_MASK_RXE) != 0U) || ((iregs[1] & (ST25R3911_IRQ_MASK_NRE >> 8U)) != 0U)) /* In rare cases there is rxs but not rxe, then we have nre */
        {
            platformLedOff( PLATFORM_LED_RX_PORT, PLATFORM_LED_RX_PIN );
        }
    #endif /* PLATFORM_LED_RX_PIN */
        
    #ifdef PLATFORM_LED_ERR_PIN

        if ( (iregs[2] & ((ST25R3911_IRQ_MASK_CRC | ST25R3911_IRQ_MASK_PAR | ST25R3911_IRQ_MASK_ERR1 | ST25R3911_IRQ_MASK_ERR2) >> 16)) != 0U)
        {
            platformLedOn( PLATFORM_LED_ERR_PORT, PLATFORM_LED_ERR_PIN );
        }
    #endif /* PLATFORM_LED_ERR_PIN */    
        
       
        irqStatus |= (uint32_t)iregs[0];
        irqStatus |= (uint32_t)iregs[1]<<8;
        irqStatus |= (uint32_t)iregs[2]<<16;
    }
    
    /* Forward all interrupts, even masked ones to application. */
    platformProtectST25RIrqStatus();
    st25r3911interrupt.status |= irqStatus;
    platformUnprotectST25RIrqStatus();
 
    /* Check received IRQs */
    st25r3911IRQCheck( irqStatus );
}


void st25r3911ModifyInterrupts(uint32_t clr_mask, uint32_t set_mask)
{
    uint8_t i;
    uint32_t old_mask;
    uint32_t new_mask;

    old_mask = st25r3911interrupt.mask;
    new_mask = (~old_mask & set_mask) | (old_mask & clr_mask);
    st25r3911interrupt.mask &= ~clr_mask;
    st25r3911interrupt.mask |= set_mask;
    for (i=0; i<3U ; i++)
    { 
        if (((new_mask >> (i*8U)) & 0xffU) == 0U) {
            continue;
        }
        st25r3911WriteRegister((ST25R3911_REG_IRQ_MASK_MAIN + i), (uint8_t)((st25r3911interrupt.mask>>(i*8U))&0xffU));
    }
    return;
}


uint32_t st25r3911WaitForInterruptsTimed(uint32_t mask, uint16_t tmo)
{
    uint32_t tmr;
    uint32_t status;
   
    tmr = platformTimerCreate(tmo);
    do 
    {
        status = (st25r3911interrupt.status & mask);
    } while( ( (!platformTimerIsExpired( tmr )) || (tmo == 0U)) && (status == 0U) );

    status = st25r3911interrupt.status & mask;
    
    platformTimerDestroy(tmr);
    
    platformProtectST25RIrqStatus();
    st25r3911interrupt.status &= ~status;
    platformUnprotectST25RIrqStatus();
    
    return status;
}

uint32_t st25r3911GetInterrupt(uint32_t mask)
{
    uint32_t irqs;

    irqs = (st25r3911interrupt.status & mask);
    if (irqs != ST25R3911_IRQ_MASK_NONE)
    {
        platformProtectST25RIrqStatus();
        st25r3911interrupt.status &= ~irqs;
        platformUnprotectST25RIrqStatus();
    }
    return irqs;
}

void st25r3911EnableInterrupts(uint32_t mask)
{
    st25r3911ModifyInterrupts(mask,0);
}

void st25r3911DisableInterrupts(uint32_t mask)
{
    st25r3911ModifyInterrupts(0,mask);
}

void st25r3911ClearInterrupts( void )
{
    uint8_t iregs[3];

    st25r3911ReadMultipleRegisters(ST25R3911_REG_IRQ_MAIN, iregs, 3);

    platformProtectST25RIrqStatus();
    st25r3911interrupt.status = 0;
    platformUnprotectST25RIrqStatus();
    return;
}

void st25r3911IRQCallbackSet( void (*cb)(void) )
{
    st25r3911interrupt.prevCallback = st25r3911interrupt.callback;
    st25r3911interrupt.callback     = cb;
}

void st25r3911IRQCallbackRestore( void )
{
    st25r3911interrupt.callback     = st25r3911interrupt.prevCallback;
    st25r3911interrupt.prevCallback = NULL;
}


static void st25r3911IRQCheck( uint32_t irqStatus )
{
    /*******************************************************************************/
    /* REMARK: Silicon workaround ST25R3911 Errata #1.6                            */
    /* Repetitive I_nre in EMV mode when I_txe is not read                         */
    /* Rarely due to timing reason, the interrupt I_nre repeats several times.     */
    /*******************************************************************************/
    
    /* If NRT has been signaled without any other IRQ */
    if( irqStatus == (ST25R3911_IRQ_MASK_NRE | ST25R3911_IRQ_MASK_TIM) )
    {
        /* If NRT was received on the last ISR as well */
        if( st25r3911interrupt.hasNRE )
        {
            /* Disabling the NRT's EMV mode and and sending a Clear command stops this condition  */
            st25r3911ClrRegisterBits( ST25R3911_REG_GPT_CONTROL, ST25R3911_REG_GPT_CONTROL_nrt_emv );
            st25r3911ExecuteCommand( ST25R3911_CMD_CLEAR_FIFO );
            st25r3911SetRegisterBits( ST25R3911_REG_GPT_CONTROL, ST25R3911_REG_GPT_CONTROL_nrt_emv );
            
            /* If RFAL is being used and a Transceive is ongoing it will fail with NRE or the  *
             * sanity timer will conclude the ongoing transceive                               */
            
            st25r3911SetRegisterBits( ST25R3911_REG_GPT_CONTROL, ST25R3911_REG_GPT_CONTROL_nrt_emv );
        }
        
        st25r3911interrupt.hasNRE = true;
    }
    else
    {
        st25r3911interrupt.hasNRE = false;
    }
    /*******************************************************************************/
}

