/*
 * Copyright (C) 2021-2022,2025 NXP Semiconductors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/* System includes */

#include <stdint.h>
#include <stdio.h>

/* Raspbian includes */
#include <uwb_logging.h>
#include <unistd.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>

/* UWB includes */
#include "driver_config.h"
#include "phUwbTypes.h"

#include "uwb_bus_interface.h"
#include "phNxpLogApis_TmlUwb.h"
#include "phOsalUwb.h"
#include "phUwb_BuildConfig.h"
#include "phUwbErrorCodes.h"

#define SPI_TRANSFER_TIMEOUT 1500
#define UWB_NODE DT_NODELABEL(uwb0)

#define SPI_OP SPI_OP_MODE_MASTER | SPI_WORD_SET(8) | SPI_TRANSFER_MSB

#define DIRECTIONAL_BYTE_LEN    0x01
#define DIRECTIONAL_BYTE_OFFSET 0x00
#define DIRECTIONAL_BYTE_WRITE  0x00
#define DIRECTIONAL_BYTE_READ   0xFF
#define HDLL_HEADER_LEN         0x02
#define HDLL_CRC_LEN            0x02
#define HDLL_LEN_MSB_MASK       0x1F
#define HDLL_RX_MAX_FRAME_LEN   64
#define HDLL_RX_MAX_DATA_LEN    (HDLL_RX_MAX_FRAME_LEN - HDLL_HEADER_LEN)

static struct spi_dt_spec spi_spec = SPI_DT_SPEC_GET(UWB_NODE, SPI_OP, 0);
static int gtransferStatus;
static uint8_t hdll_payload_rx[HDLL_RX_MAX_DATA_LEN + DIRECTIONAL_BYTE_LEN];
static uint8_t hdll_payload_tx[HDLL_RX_MAX_DATA_LEN + DIRECTIONAL_BYTE_LEN];

/* This semaphore is signaled when SPI write is completed successfully*/
void *mSpiTransferSem = NULL;

static volatile void *spi_async_userdata;

static void spi_MasterCallback(const struct device *dev, int status, void *userdata)
{
    gtransferStatus = status;
    (void)phOsalUwb_ProduceSemaphore(mSpiTransferSem);
}

static int uwb_spi_manual_cs_set(uwb_bus_board_ctx_t *pCtx, int active)
{
    if (pCtx == NULL || pCtx->cs_gpio == NULL || pCtx->cs_gpio->port == NULL) {
        return 0;
    }

    return gpio_pin_set_dt(pCtx->cs_gpio, active);
}

uwb_bus_status_t uwb_bus_init(uwb_bus_board_ctx_t *pCtx)
{
    if (pCtx == NULL) {
        NXPLOG_UWB_TML_E("uwbs bus context is NULL");
        return kUWB_bus_Status_FAILED;
    }
    pCtx->masterHandle = &spi_spec;

    if (!spi_is_ready_dt(pCtx->masterHandle)) {
        NXPLOG_UWB_TML_E("Error: SPI device is not ready");
        return kUWB_bus_Status_FAILED;
    }

    /* This semaphore is signaled when SPI data is send out completely Rhodes*/
    if (phOsalUwb_CreateBinSem(&mSpiTransferSem) != UWBSTATUS_SUCCESS) {
        LOG_E("Error: uwb_bus_init(), could not create semaphore mSpiTransferSem\n");
        return kUWB_bus_Status_FAILED;
    }

    /*This semaphore is signaled in the ISR context.*/
    if (phOsalUwb_CreateSemaphore(&pCtx->mIrqWaitSem, 0) != kUWBSTATUS_SUCCESS) {
        LOG_E("Error: uwb_uwbs_tml_init(), could not create semaphore mWaitIrqSem\n");
        return kUWB_bus_Status_FAILED;
    }

    if (uwb_bus_io_init(pCtx) != kUWB_bus_Status_OK) {
        LOG_E("Error: uwb_bus_init(), could not initialize UWB GPIOs\n");
        return kUWB_bus_Status_FAILED;
    }

    LOG_D("uwb_bus_init Done");
    return kUWB_bus_Status_OK;
}

uwb_bus_status_t uwb_bus_data_tx(uwb_bus_board_ctx_t *pCtx, uint8_t *pBuf, size_t bufLen)
{
    uwb_bus_status_t bus_status = kUWB_bus_Status_FAILED;
    int status                  = -1;
    struct spi_buf spi_buffers[2] = {0};
    size_t data_size;

    if (pCtx == NULL) {
        NXPLOG_UWB_TML_E("uwbs bus context is NULL");
        goto end;
    }

    if (pBuf == NULL || bufLen == 0) {
        goto end;
    }
    LOG_D("uwb_bus_data_tx bufLen = %d", bufLen);
    /* Set direction byte as Host write */
    pBuf[DIRECTIONAL_BYTE_OFFSET] = DIRECTIONAL_BYTE_WRITE;
    data_size                     = bufLen;

    spi_buffers[0].buf = pBuf;
    spi_buffers[0].len = data_size + DIRECTIONAL_BYTE_LEN;
    spi_buffers[1].buf = NULL;

    const struct spi_buf_set tx_buff = {
        .buffers = &spi_buffers[0],
        .count   = 1,
    };

    const struct spi_buf_set rx_buff = {
        .buffers = &spi_buffers[1],
        .count   = 1,
    };

    if (uwb_spi_manual_cs_set(pCtx, 1) != 0) {
        LOG_E("%s : failed to assert chip-select", __FUNCTION__);
        goto end;
    }

    status = spi_transceive_cb(pCtx->masterHandle->bus,
        &pCtx->masterHandle->config,
        &tx_buff,
        &rx_buff,
        spi_MasterCallback,
        (void *)spi_async_userdata);
    if (status == 0) {
        if (phOsalUwb_ConsumeSemaphore_WithTimeout(mSpiTransferSem, SPI_TRANSFER_TIMEOUT) != UWBSTATUS_SUCCESS) {
            LOG_E("%s : spi transfer timeout", __FUNCTION__);
            (void)uwb_spi_manual_cs_set(pCtx, 0);
            goto end;
        }
        (void)uwb_spi_manual_cs_set(pCtx, 0);
        if (gtransferStatus != 0) {
            goto end;
        }
        bus_status = kUWB_bus_Status_OK;
    }
    else {
        (void)uwb_spi_manual_cs_set(pCtx, 0);
    }
end:
    return bus_status;
}

uwb_bus_status_t uwb_bus_data_rx(uwb_bus_board_ctx_t *pCtx, uint8_t *pBuf, size_t bufLen)
{
    uwb_bus_status_t bus_status = kUWB_bus_Status_FAILED;
    int status                  = -1;
    struct spi_buf spi_buffers[2];
    uint8_t DataBuff[2] = {0};

    if (pCtx == NULL) {
        NXPLOG_UWB_TML_E("uwbs bus context is NULL");
        goto end;
    }

    if (pBuf == NULL || bufLen == 0) {
        NXPLOG_UWB_TML_E("uwb_bus_data_rx failed");
        goto end;
    }
    LOG_D("uwb_bus_data_rx bufLen = %d", bufLen);
    /* Set directional byte for read operation */
    DataBuff[DIRECTIONAL_BYTE_OFFSET] = DIRECTIONAL_BYTE_READ;
    spi_buffers[0].buf                = DataBuff;
    spi_buffers[0].len                = DIRECTIONAL_BYTE_LEN;

    spi_buffers[1].buf = pBuf;
    spi_buffers[1].len = bufLen + DIRECTIONAL_BYTE_LEN;

    const struct spi_buf_set tx_buff = {
        .buffers = &spi_buffers[0],
        .count   = 1,
    };

    const struct spi_buf_set rx_buff = {
        .buffers = &spi_buffers[1],
        .count   = 1,
    };

    if (uwb_spi_manual_cs_set(pCtx, 1) != 0) {
        LOG_E("%s : failed to assert chip-select", __FUNCTION__);
        goto end;
    }

    status = spi_transceive_cb(pCtx->masterHandle->bus,
        &pCtx->masterHandle->config,
        &tx_buff,
        &rx_buff,
        spi_MasterCallback,
        (void *)spi_async_userdata);
    if (status == 0) {
        if (phOsalUwb_ConsumeSemaphore_WithTimeout(mSpiTransferSem, SPI_TRANSFER_TIMEOUT) != UWBSTATUS_SUCCESS) {
            LOG_E("%s : spi transfer timeout", __FUNCTION__);
            (void)uwb_spi_manual_cs_set(pCtx, 0);
            goto end;
        }
        (void)uwb_spi_manual_cs_set(pCtx, 0);
        if (gtransferStatus != 0) {
            goto end;
        }
        LOG_MAU8_D("uwb_bus_data_rx raw", pBuf, bufLen + DIRECTIONAL_BYTE_LEN);
        bus_status = kUWB_bus_Status_OK;
    }
    else {
        (void)uwb_spi_manual_cs_set(pCtx, 0);
    }
end:
    return bus_status;
}

uwb_bus_status_t uwb_bus_data_rx_hdll(uwb_bus_board_ctx_t *pCtx, uint8_t *pBuf, size_t pBufSize, size_t *pFrameLen)
{
    uwb_bus_status_t bus_status = kUWB_bus_Status_FAILED;
    int status                  = -1;
    uint8_t read_cmd            = DIRECTIONAL_BYTE_READ;
    uint8_t header_rx[DIRECTIONAL_BYTE_LEN + HDLL_HEADER_LEN] = {0};
    uint16_t payload_len                                      = 0;
    size_t payload_crc_len                                    = 0;
    size_t payload_read_len                                   = 0;
    size_t frame_len                                          = 0;

    if (pCtx == NULL || pBuf == NULL || pFrameLen == NULL) {
        NXPLOG_UWB_TML_E("uwb_bus_data_rx_hdll failed");
        goto end;
    }

    const struct spi_buf header_tx_buf = {
        .buf = &read_cmd,
        .len = sizeof(read_cmd),
    };
    struct spi_buf header_rx_buf = {
        .buf = header_rx,
        .len = sizeof(header_rx),
    };
    const struct spi_buf_set header_tx = {
        .buffers = &header_tx_buf,
        .count   = 1,
    };
    const struct spi_buf_set header_rx_set = {
        .buffers = &header_rx_buf,
        .count   = 1,
    };

    if (uwb_spi_manual_cs_set(pCtx, 1) != 0) {
        LOG_E("%s : failed to assert chip-select", __FUNCTION__);
        goto end;
    }

    gtransferStatus = -1;
    status = spi_transceive_cb(pCtx->masterHandle->bus,
        &pCtx->masterHandle->config,
        &header_tx,
        &header_rx_set,
        spi_MasterCallback,
        (void *)spi_async_userdata);
    if (status != 0) {
        goto release_cs;
    }
    if (phOsalUwb_ConsumeSemaphore_WithTimeout(mSpiTransferSem, SPI_TRANSFER_TIMEOUT) != UWBSTATUS_SUCCESS) {
        LOG_E("%s : HDLL header spi transfer timeout", __FUNCTION__);
        goto release_cs;
    }
    if (gtransferStatus != 0) {
        goto release_cs;
    }

    LOG_MAU8_D("uwb_bus_data_rx_hdll header", header_rx, sizeof(header_rx));

    payload_len = (uint16_t)(((header_rx[DIRECTIONAL_BYTE_LEN] & HDLL_LEN_MSB_MASK) << 8) |
                             header_rx[DIRECTIONAL_BYTE_LEN + 1]);
    payload_crc_len = payload_len + HDLL_CRC_LEN;
    payload_read_len = payload_crc_len + DIRECTIONAL_BYTE_LEN;
    frame_len       = HDLL_HEADER_LEN + payload_crc_len;
    if ((frame_len + DIRECTIONAL_BYTE_LEN + 1) > pBufSize || payload_read_len > sizeof(hdll_payload_rx)) {
        LOG_E("%s : HDLL frame too large: %u", __FUNCTION__, (unsigned int)frame_len);
        goto release_cs;
    }

    if (payload_crc_len > 0) {
        phOsalUwb_SetMemory(hdll_payload_tx, DIRECTIONAL_BYTE_READ, payload_read_len);
        phOsalUwb_SetMemory(hdll_payload_rx, 0, payload_read_len);

        const struct spi_buf payload_tx_buf = {
            .buf = hdll_payload_tx,
            .len = payload_read_len,
        };
        struct spi_buf payload_rx_buf = {
            .buf = hdll_payload_rx,
            .len = payload_read_len,
        };
        const struct spi_buf_set payload_tx_set = {
            .buffers = &payload_tx_buf,
            .count   = 1,
        };
        const struct spi_buf_set payload_rx_set = {
            .buffers = &payload_rx_buf,
            .count   = 1,
        };

        gtransferStatus = -1;
        status = spi_transceive_cb(pCtx->masterHandle->bus,
            &pCtx->masterHandle->config,
            &payload_tx_set,
            &payload_rx_set,
            spi_MasterCallback,
            (void *)spi_async_userdata);
        if (status != 0) {
            goto release_cs;
        }
        if (phOsalUwb_ConsumeSemaphore_WithTimeout(mSpiTransferSem, SPI_TRANSFER_TIMEOUT) != UWBSTATUS_SUCCESS) {
            LOG_E("%s : HDLL payload spi transfer timeout", __FUNCTION__);
            goto release_cs;
        }
        if (gtransferStatus != 0) {
            goto release_cs;
        }

        LOG_MAU8_D("uwb_bus_data_rx_hdll payload", &hdll_payload_rx[DIRECTIONAL_BYTE_LEN], payload_crc_len);
    }

    pBuf[DIRECTIONAL_BYTE_OFFSET] = header_rx[DIRECTIONAL_BYTE_OFFSET];
    pBuf[DIRECTIONAL_BYTE_LEN]    = header_rx[DIRECTIONAL_BYTE_LEN];
    pBuf[DIRECTIONAL_BYTE_LEN + 1] = header_rx[DIRECTIONAL_BYTE_LEN + 1];
    pBuf[DIRECTIONAL_BYTE_LEN + HDLL_HEADER_LEN] = DIRECTIONAL_BYTE_READ;
    phOsalUwb_MemCopy(&pBuf[DIRECTIONAL_BYTE_LEN + HDLL_HEADER_LEN + 1],
        &hdll_payload_rx[DIRECTIONAL_BYTE_LEN],
        payload_crc_len);

    *pFrameLen = frame_len;
    bus_status = kUWB_bus_Status_OK;

release_cs:
    (void)uwb_spi_manual_cs_set(pCtx, 0);
end:
    return bus_status;
}

uwb_bus_status_t uwb_bus_deinit(uwb_bus_board_ctx_t *pCtx)
{
    if (pCtx == NULL) {
        LOG_E("uwbs bus context is NULL");
        return kUWB_bus_Status_FAILED;
    }

    phOsalUwb_DeleteSemaphore(&mSpiTransferSem);
    phOsalUwb_ProduceSemaphore(pCtx->mIrqWaitSem);
    phOsalUwb_Delay(2);
    phOsalUwb_DeleteSemaphore(&pCtx->mIrqWaitSem);
    mSpiTransferSem = NULL;
    phOsalUwb_SetMemory(pCtx, 0, sizeof(uwb_bus_board_ctx_t));
    return kUWB_bus_Status_OK;
}

uwb_bus_status_t uwb_bus_reset(uwb_bus_board_ctx_t *pCtx)
{
    if (pCtx == NULL) {
        LOG_E("uwbs bus context is NULL");
        return kUWB_bus_Status_FAILED;
    }

    if (pCtx->rst_gpio == NULL || pCtx->rst_gpio->port == NULL) {
        return kUWB_bus_Status_OK;
    }

    if (gpio_pin_set_dt(pCtx->rst_gpio, 0) != 0) {
        LOG_E("UWBD reset GPIO set low failed");
        return kUWB_bus_Status_FAILED;
    }

    k_sleep(K_MSEC(10));

    if (gpio_pin_set_dt(pCtx->rst_gpio, 1) != 0) {
        LOG_E("UWBD reset GPIO set high failed");
        return kUWB_bus_Status_FAILED;
    }

    k_sleep(K_MSEC(20));

    return kUWB_bus_Status_OK;
}

void uwb_port_DelayinMicroSec(int delay)
{
    k_sleep(K_USEC(delay));
}

uwb_bus_status_t uwb_bus_data_trx(uwb_bus_board_ctx_t *pCtx, uint8_t *pBuf, size_t bufLen)
{
    LOG_W("uwb_bus_data_trx");
    uwb_bus_status_t bus_status = kUWB_bus_Status_FAILED;
    int status                  = -1;
    struct spi_buf spi_buffers[2];
    uint8_t DataBuff[2] = {0};

    if (pCtx == NULL) {
        NXPLOG_UWB_TML_E("uwbs bus context is NULL");
        goto end;
    }

    if (pBuf == NULL || bufLen == 0) {
        NXPLOG_UWB_TML_E("uwb_bus_data_trx failed");
        goto end;
    }

    /* Data Receive */
    if (pCtx->op_mode == READ_MODE) {
        LOG_W("uwb_bus_data_trx : READ_MODE ");
        /* set direction as Host Read */
        DataBuff[DIRECTIONAL_BYTE_OFFSET] = DIRECTIONAL_BYTE_READ;
        spi_buffers[0].buf                = DataBuff;
        spi_buffers[0].len                = DIRECTIONAL_BYTE_LEN;

        spi_buffers[1].buf = &pBuf[0];
        spi_buffers[1].len = bufLen + DIRECTIONAL_BYTE_LEN;

        const struct spi_buf_set tx_buff = {
            .buffers = &spi_buffers[0],
            .count   = 1,
        };

        const struct spi_buf_set rx_buff = {
            .buffers = &spi_buffers[1],
            .count   = 1,
        };

        if (uwb_spi_manual_cs_set(pCtx, 1) != 0) {
            LOG_E("%s : failed to assert chip-select", __FUNCTION__);
            goto end;
        }

        status = spi_transceive_cb(pCtx->masterHandle->bus,
            &pCtx->masterHandle->config,
            &tx_buff,
            &rx_buff,
            spi_MasterCallback,
            (void *)spi_async_userdata);
        if (status == 0) {
            if (phOsalUwb_ConsumeSemaphore_WithTimeout(mSpiTransferSem, MAX_UWBS_SPI_TRANSFER_TIMEOUT) !=
                UWBSTATUS_SUCCESS) {
                LOG_E("%s : spi transfer timeout", __FUNCTION__);
                (void)uwb_spi_manual_cs_set(pCtx, 0);
                goto end;
            }
            (void)uwb_spi_manual_cs_set(pCtx, 0);
            if (gtransferStatus != 0) {
                goto end;
            }
            bus_status = kUWB_bus_Status_OK;
        }
        else {
            (void)uwb_spi_manual_cs_set(pCtx, 0);
        }
    }
    /* Data Transmit */
    else if (pCtx->op_mode == WRITE_MODE) {
        LOG_W("uwb_bus_data_trx : WRITE_MODE ");
        spi_buffers[0].buf = &pBuf[0];
        spi_buffers[0].len = bufLen + DIRECTIONAL_BYTE_LEN;
        spi_buffers[1].buf = NULL;

        const struct spi_buf_set tx_buff = {
            .buffers = &spi_buffers[0],
            .count   = 1,
        };

        const struct spi_buf_set rx_buff = {
            .buffers = &spi_buffers[1],
            .count   = 1,
        };

        if (uwb_spi_manual_cs_set(pCtx, 1) != 0) {
            LOG_E("%s : failed to assert chip-select", __FUNCTION__);
            goto end;
        }

        status = spi_transceive_cb(pCtx->masterHandle->bus,
            &pCtx->masterHandle->config,
            &tx_buff,
            &rx_buff,
            spi_MasterCallback,
            (void *)spi_async_userdata);
        if (status == 0) {
            if (phOsalUwb_ConsumeSemaphore_WithTimeout(mSpiTransferSem, MAX_UWBS_SPI_TRANSFER_TIMEOUT) !=
                UWBSTATUS_SUCCESS) {
                LOG_E("%s : spi transfer timeout", __FUNCTION__);
                (void)uwb_spi_manual_cs_set(pCtx, 0);
                goto end;
            }
            (void)uwb_spi_manual_cs_set(pCtx, 0);
            if (gtransferStatus != 0) {
                goto end;
            }
            bus_status = kUWB_bus_Status_OK;
        }
        else {
            (void)uwb_spi_manual_cs_set(pCtx, 0);
        }
    }
    else {
        /* Invalid mode  */
        LOG_E("Invalid Mode");
    }

end:
    return bus_status;
}
