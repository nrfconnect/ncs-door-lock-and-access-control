/* Copyright 2021-2022,2025 NXP
 *
 * NXP Confidential. This software is owned or controlled by NXP and may only
 * be used strictly in accordance with the applicable license terms.  By
 * expressly accepting such terms or by downloading, installing, activating
 * and/or otherwise using the software, you are agreeing that you have read,
 * and that you agree to comply with and are bound by, such license terms.  If
 * you do not agree to be bound by the applicable license terms, then you may
 * not retain, install, activate or otherwise use the software.
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdint.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <uwb_bus_interface.h>
#include "phNxpLogApis_Board.h"
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <UWB_GPIOExtender.h>

#define UWB_NUM_INTERRUPTS 1
#define UWB_NODE DT_NODELABEL(uwb0)
#define UWB_IRQ_POLL_INTERVAL_MS 1
#define UWB_RST_ASSERT_MS 50
#define UWB_RST_RELEASE_MS 150

static struct gpio_dt_spec irq_gpio = GPIO_DT_SPEC_GET(UWB_NODE, uwb_irq_gpios);
static struct gpio_dt_spec cs_gpio = GPIO_DT_SPEC_GET_OR(UWB_NODE, uwb_cs_gpios, {0});
static struct gpio_dt_spec rst_gpio = GPIO_DT_SPEC_GET_OR(UWB_NODE, uwb_rstn_gpios, {0});

static struct gpio_callback irq_gpio_cb;
static uwbs_io_callback mCallbacks[UWB_NUM_INTERRUPTS];

static void uwb_bus_io_generic_cb()
{
    /* Handle more callbacks if needed */
    if (mCallbacks[0].fn) {
        mCallbacks[0].fn(mCallbacks[0].args);
    }
}

void UWB_HELIOS_IRQ_HANDLER(const struct device *unused1, struct gpio_callback *unused2, uint32_t unused3)
{
    uwb_bus_io_generic_cb();
}

void uwb_bus_io_irq_cb(void *args)
{
    uwb_bus_board_ctx_t *pCtx = (uwb_bus_board_ctx_t *)args;
    // Signal TML read task
    phOsalUwb_ProduceSemaphore(pCtx->mIrqWaitSem);
}

uwb_bus_status_t uwb_bus_io_deinit(uwb_bus_board_ctx_t *pCtx)
{
    phOsalUwb_SetMemory(mCallbacks, 0, sizeof(uwbs_io_callback) * UWB_NUM_INTERRUPTS);
    return kUWB_bus_Status_OK;
}

uwb_bus_status_t uwb_bus_io_init(uwb_bus_board_ctx_t *pCtx)
{
    uwb_bus_status_t status = kUWB_bus_Status_FAILED;
    int ret                 = 0;

    if (pCtx == NULL) {
        LOG_E("uwbs bus context is NULL");
        return status;
    }

    pCtx->irq_gpio = &irq_gpio;
    pCtx->cs_gpio  = &cs_gpio;
    pCtx->rst_gpio = &rst_gpio;

    if (pCtx->cs_gpio->port != NULL) {
        if (!gpio_is_ready_dt(pCtx->cs_gpio)) {
            LOG_E("UWBD chip-select GPIO is not ready");
            return status;
        }

        ret = gpio_pin_configure_dt(pCtx->cs_gpio, GPIO_OUTPUT_INACTIVE);
        if (ret != 0) {
            LOG_E("UWBD chip-select GPIO Pin configuration failed");
            return status;
        }

        ret = gpio_pin_set_dt(pCtx->cs_gpio, 0);
        if (ret != 0) {
            LOG_E("UWBD chip-select GPIO Pin set inactive failed");
            return status;
        }
    }

    if (pCtx->rst_gpio->port != NULL) {
        if (!gpio_is_ready_dt(pCtx->rst_gpio)) {
            LOG_E("UWBD reset GPIO is not ready");
            return status;
        }

        ret = gpio_pin_configure_dt(pCtx->rst_gpio, GPIO_OUTPUT_INACTIVE);
        if (ret != 0) {
            LOG_E("UWBD reset GPIO Pin configuration failed");
            return status;
        }

        ret = gpio_pin_set_dt(pCtx->rst_gpio, 0);
        if (ret != 0) {
            LOG_E("UWBD reset GPIO Pin assert failed");
            return status;
        }
        k_msleep(UWB_RST_ASSERT_MS);

        ret = gpio_pin_set_dt(pCtx->rst_gpio, 1);
        if (ret != 0) {
            LOG_E("UWBD reset GPIO Pin set high failed");
            return status;
        }
        k_msleep(UWB_RST_RELEASE_MS);
    }

    status         = uwb_bus_io_uwbs_irq_enable(pCtx);

    return status;
}

uwb_bus_status_t uwb_bus_io_val_set(uwb_bus_board_ctx_t *pCtx, uwbs_io_t gpioPin, uwbs_io_state_t gpioValue)
{
    if (pCtx == NULL) {
        LOG_E("uwbs bus context is NULL");
        return kUWB_bus_Status_FAILED;
    }
    if (kUWBS_IO_State_NA == gpioValue) {
        LOG_E("Invalid GPIO state");
        return kUWB_bus_Status_FAILED;
    }

    switch (gpioPin) {
    case kUWBS_IO_O_RSTN: {
        if (pCtx->rst_gpio != NULL && pCtx->rst_gpio->port != NULL) {
            if (!gpio_is_ready_dt(pCtx->rst_gpio)) {
                LOG_E("UWBD reset GPIO is not ready");
                return kUWB_bus_Status_FAILED;
            }

            if (gpio_pin_set_dt(pCtx->rst_gpio, gpioValue) != 0) {
                LOG_E("UWBD reset GPIO Pin set failed");
                return kUWB_bus_Status_FAILED;
            }
            if (gpioValue == kUWBS_IO_State_Low) {
                k_msleep(UWB_RST_ASSERT_MS);
            }
            else {
                k_msleep(UWB_RST_RELEASE_MS);
            }
        }
        else {
            LOG_E("UWBD reset GPIO is not configured in devicetree");
            return kUWB_bus_Status_FAILED;
        }
        return kUWB_bus_Status_OK;
    } break;

    default:
        LOG_E("UWBD IO GPIO Pin not supported");
        return kUWB_bus_Status_FAILED;
    }
}

uwb_bus_status_t uwb_bus_io_val_get(uwb_bus_board_ctx_t *pCtx, uwbs_io_t gpioPin, uwbs_io_state_t *pGpioValue)
{
    if (pCtx == NULL || pGpioValue == NULL) {
        LOG_E("uwbs bus context is NULL");
        return kUWB_bus_Status_FAILED;
    }
    switch (gpioPin) {
    case kUWBS_IO_I_UWBS_IRQ: {
        *pGpioValue = (uwbs_io_state_t)gpio_pin_get_dt(pCtx->irq_gpio);
    } break;

    default:
        LOG_E("UWBD IO GPIO Pin not supported");
        return kUWB_bus_Status_FAILED;
    }
    return kUWB_bus_Status_OK;
}

uwb_bus_status_t uwb_bus_io_irq_dis(uwb_bus_board_ctx_t *pCtx, uwbs_io_t irqPin)
{
    uwb_bus_status_t status = kUWB_bus_Status_FAILED;

    if (pCtx == NULL) {
        LOG_E("uwbs bus context is NULL");
        return status;
    }

    switch (irqPin) {
    case kUWBS_IO_I_UWBS_IRQ: {
    } break;
    default:
        LOG_E("UWBD IO GPIO Pin Interrupt not supported");
        goto end;
    }

    mCallbacks[0].fn   = NULL;
    mCallbacks[0].args = NULL;
    status             = kUWB_bus_Status_OK;
end:
    return status;
}

uwb_bus_status_t uwb_bus_io_irq_en(uwb_bus_board_ctx_t *pCtx, uwbs_io_t irqPin, uwbs_io_callback *pCallback)
{
    uwb_bus_status_t status = kUWB_bus_Status_FAILED;
    int ret                 = 0;

    if (pCtx == NULL || pCallback == NULL) {
        LOG_E("uwbs bus context is NULL");
        return status;
    }

    switch (irqPin) {
    case kUWBS_IO_I_UWBS_IRQ: {
        mCallbacks[0].fn   = pCallback->fn;
        mCallbacks[0].args = pCallback->args;
    } break;
    default:
        LOG_E("UWBD IO GPIO Pin Interrupt not supported");
        goto end;
    }

    if (!gpio_is_ready_dt(pCtx->irq_gpio)) {
        LOG_E("UWBD IO GPIO is not ready");
        goto end;
    }

    /* Configure IRQ pin and register the callback */
    ret = gpio_pin_configure_dt(pCtx->irq_gpio, GPIO_INPUT);
    if (ret != 0) {
        LOG_E("UWBD IO GPIO Pin configuration failed");
        goto end;
    }
    /* Configure the interrupt edge for IRQ pin */
    ret = gpio_pin_interrupt_configure_dt(pCtx->irq_gpio, GPIO_INT_EDGE_RISING);
    if (ret != 0) {
        LOG_E("UWBD IO GPIO Pin interrupt configuration failed");
        goto end;
    }

    gpio_init_callback(&irq_gpio_cb, UWB_HELIOS_IRQ_HANDLER, BIT(pCtx->irq_gpio->pin));
    gpio_add_callback(pCtx->irq_gpio->port, &irq_gpio_cb);

    status = kUWB_bus_Status_OK;
end:
    return status;
}

uwb_bus_status_t uwb_bus_io_irq_wait(uwb_bus_board_ctx_t *pCtx, uint32_t timeout_ms)
{
    uwb_bus_status_t status = kUWB_bus_Status_OK;
    uint32_t elapsed_ms     = 0;
    int irq_level           = 0;

    if (pCtx == NULL) {
        LOG_E("uwbs bus context is NULL");
        return kUWB_bus_Status_FAILED;
    }

    if (pCtx->irq_gpio == NULL || !gpio_is_ready_dt(pCtx->irq_gpio)) {
        LOG_E("UWBD IO GPIO is not ready");
        return kUWB_bus_Status_FAILED;
    }

    irq_level = gpio_pin_get_dt(pCtx->irq_gpio);
    LOG_D("uwb_bus_io_irq_wait: initial IRQ level=%d", irq_level);
    if (irq_level > 0) {
        return kUWB_bus_Status_OK;
    }

    while (phOsalUwb_ConsumeSemaphore_WithTimeout(pCtx->mIrqWaitSem, 0) == UWBSTATUS_SUCCESS) {
        LOG_D("uwb_bus_io_irq_wait: drained stale IRQ semaphore");
    }

    while (elapsed_ms < timeout_ms) {
        if (phOsalUwb_ConsumeSemaphore_WithTimeout(pCtx->mIrqWaitSem, UWB_IRQ_POLL_INTERVAL_MS) ==
            UWBSTATUS_SUCCESS) {
            irq_level = gpio_pin_get_dt(pCtx->irq_gpio);
            LOG_D("uwb_bus_io_irq_wait: IRQ semaphore after %u ms, level=%d", elapsed_ms, irq_level);
            if (irq_level > 0) {
                return kUWB_bus_Status_OK;
            }
        }

        irq_level = gpio_pin_get_dt(pCtx->irq_gpio);
        if (irq_level > 0) {
            LOG_D("uwb_bus_io_irq_wait: IRQ high after polling %u ms", elapsed_ms);
            return kUWB_bus_Status_OK;
        }

        elapsed_ms += UWB_IRQ_POLL_INTERVAL_MS;
    }

    irq_level = gpio_pin_get_dt(pCtx->irq_gpio);
    if (irq_level <= 0) {
        LOG_D("uwb_bus_io_irq_wait: timeout, final IRQ level=%d", irq_level);
        status = kUWB_bus_Status_FAILED;
    }

    return status;
}

uwb_bus_status_t uwb_bus_io_uwbs_irq_enable(uwb_bus_board_ctx_t *pCtx)
{
    uwbs_io_callback callback;
    if (pCtx == NULL) {
        LOG_E("uwbs bus context is NULL");
        return kUWB_bus_Status_FAILED;
    }
    callback.fn   = uwb_bus_io_irq_cb;
    callback.args = pCtx;
    return uwb_bus_io_irq_en(pCtx, kUWBS_IO_I_UWBS_IRQ, &callback);
}
