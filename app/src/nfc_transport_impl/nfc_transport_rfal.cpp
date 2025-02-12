/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "nfc_transport_rfal.h"
#include <rfal_ncs_pal.h>
#include <rfal_nfc_config.h>

#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(nfc_st_rfal_impl, CONFIG_NCS_ALIRO_RFAL_LOG_LEVEL);

namespace Aliro {

K_THREAD_STACK_DEFINE(mStack, CONFIG_RFAL_WORKER_THREAD_STACK_SIZE);

extern "C" struct k_sem irq_sem;

void NfcTransportRfal::Run()
{
	while (true) {
		if (mTimeout) {
			VerifyAndCall(Instance().NfcDriver::mCallbacks.mOnError, ALIRO_TIMEOUT);
			mTimeout = false;
		}
		rfalNfcWorker();
		k_sem_take(&irq_sem, K_MSEC(CONFIG_RFAL_NFC_WORKER_TIMEOUT_MS));
	}
}

void NfcTransportRfal::RfalNotifyCallback(rfalNfcState state)
{
	switch (state) {
	case RFAL_NFC_STATE_WAKEUP_MODE:
		LOG_DBG("RFAL: Wake Up mode state");
		break;
	case RFAL_NFC_STATE_POLL_TECHDETECT:
		LOG_DBG("RFAL: Poll technology detect state");
		if (mNfcConfig.wakeupEnabled) {
			LOG_DBG("RFAL: Wake Up mode terminated. Polling for devices.");
		}
		break;
	case RFAL_NFC_STATE_POLL_SELECT:
		LOG_DBG("RFAL: Poll select state");
		// Check if in case of multiple devices, selection is already attempted
		if (!mMultiSel) {
			mMultiSel = true;
			// Multiple devices were found, activate first of them
			uint8_t devCnt;
			rfalNfcDevice *dev;
			rfalNfcGetDevicesFound(&dev, &devCnt);
			rfalNfcSelect(0);
			LOG_DBG("RFAL: Multiple Tags detected: %d", devCnt);
		} else {
			rfalNfcDeactivate(RFAL_NFC_DEACTIVATE_DISCOVERY);
		}
		break;
	case RFAL_NFC_STATE_START_DISCOVERY:
		LOG_DBG("RFAL: Start discovery state");
		mMultiSel = false;
		break;
	case RFAL_NFC_STATE_DATAEXCHANGE:
		LOG_DBG("RFAL: Data exchange state");
		break;
	case RFAL_NFC_STATE_DATAEXCHANGE_DONE:
		k_timer_stop(&mRxTimer);
		CaptureRxData();
		k_timer_start(&mIdleTimer, K_MSEC(sIdleTimerTimeoutMs), K_NO_WAIT);
		break;
	case RFAL_NFC_STATE_DEACTIVATION:
		LOG_DBG("RFAL: Deactivation State");
		break;
	case RFAL_NFC_STATE_ACTIVATED:
		LOG_DBG("RFAL: Activated state");
		SelectActiveTag();
		break;
	default:
		break;
	}
}

ReturnCode NfcTransportRfal::RfalNfcInit()
{
	ReturnCode err = rfalNfcInitialize();
	VerifyOrReturnStatus(!err, err);

	// Set default discovery parameters.
	rfalNfcDefaultDiscParams(&mNfcConfig);
	// Set wake-up configuration.
	rfalNfcWakeupConfig(&mNfcConfig);
	// Set only NFC-A technology Flag.
	mNfcConfig.techs2Find |= RFAL_NFC_POLL_TECH_A;

	mNfcConfig.notifyCb = [](rfalNfcState state) { Instance().RfalNotifyCallback(state); };

	return err;
}

void NfcTransportRfal::SelectActiveTag()
{
	rfalNfcDevice *nfcDevice;
	rfalNfcGetActiveDevice(&nfcDevice);
	VerifyOrReturn(nfcDevice);

	LOG_INF("RFAL: Active device type = %d", nfcDevice->type);

	if (nfcDevice->type == RFAL_NFC_LISTEN_TYPE_NFCA) {
		switch (nfcDevice->dev.nfca.type) {
		case RFAL_NFCA_T4T:
			LOG_HEXDUMP_DBG(nfcDevice->nfcid, nfcDevice->nfcidLen,
					"RFAL: NFCA Passive ISO-DEP device found. UID: ");
			VerifyAndCall(this->NfcDriver::mCallbacks.mOnTagDetected);
			return;
		default:
			LOG_HEXDUMP_DBG(nfcDevice->nfcid, nfcDevice->nfcidLen,
					"RFAL: Usupported NFC card found. UID: %s");
			break;
		}
	}
}

void NfcTransportRfal::CaptureRxData()
{
	ReturnCode status = rfalNfcDataExchangeGetStatus();
	if (status == RFAL_ERR_BUSY) {
		LOG_ERR("RFAL: Data transaction has not been completed [status: %d]", status);
		VerifyAndCall(this->NfcDriver::mCallbacks.mOnError, ALIRO_INVALID_STATE);
		return;
	}

	uint16_t currentDataLen = *mRcvLen;
	memset(mRxBuffer.data(), 0, mRxBuffer.size());

	VerifyOrReturn(mRxBuffer.size() >= currentDataLen);
	memcpy(mRxBuffer.data(), mRxData, currentDataLen);

	LOG_HEXDUMP_DBG(mRxBuffer.data(), currentDataLen, "RFAL: RX data:");

	VerifyAndCall(NfcDriver::mCallbacks.mOnDataReceived, { .mData = mRxBuffer.data(), .mLength = currentDataLen },
		      0);
}

/*
******************************************************************************
* IsoDep interface implementation
******************************************************************************
*/
AliroError NfcTransportRfal::_Init(IsoDep::Callbacks callbacks)
{
	IsoDep::mCallbacks = callbacks;
	return ALIRO_NO_ERROR;
}

AliroError NfcTransportRfal::_PrepareData(NfcTransport::Data data)
{
	// ISO-DEP layer is implemented internally in the RFAL, no need for special data handling
	return ALIRO_ERROR_NOT_IMPLEMENTED;
}

AliroError NfcTransportRfal::_PrepareRats()
{
	// RATS is sent by the driver internally as an activation procedure, so we can presume the tag is fully selected
	// a this point
	return ALIRO_ERROR_NOT_IMPLEMENTED;
}

AliroError NfcTransportRfal::_HandleReceivedData(NfcTransport::Data data, int transferError)
{
	// No specific processing needed, all ISO-DEP specific data handling happens in driver's internals
	return ALIRO_ERROR_NOT_IMPLEMENTED;
}

AliroError NfcTransportRfal::_ReportTimeout()
{
	// No special handling needed with RFAL
	return ALIRO_ERROR_NOT_IMPLEMENTED;
}

AliroError NfcTransportRfal::_DeselectTag()
{
	ReturnCode err = rfalNfcDeactivate(RFAL_NFC_DEACTIVATE_SLEEP);
	return err ? ALIRO_INVALID_STATE : ALIRO_NO_ERROR;
}

/* Implementation of the generic IsoDep instance getter. */
IsoDep &IsoDepInstance()
{
	return NfcTransportRfal::Instance();
}
/*
******************************************************************************
******************************************************************************
*/

/*
******************************************************************************
* NfcDriver interface implementation
******************************************************************************
*/
AliroError NfcTransportRfal::_Init(NfcDriver::Callbacks callbacks)
{
	NfcDriver::mCallbacks = callbacks;

	int err = rfal_ncs_pal_init();
	if (err) {
		LOG_ERR("RFAL: NFC PAL init failed %d", err);
		return ALIRO_ERROR_INTERNAL;
	}

	k_tid_t thread = k_thread_create(
		&mThread, mStack, CONFIG_RFAL_WORKER_THREAD_STACK_SIZE,
		[](void *, void *, void *) { return Instance().Run(); }, nullptr, nullptr, nullptr,
		K_PRIO_PREEMPT(CONFIG_RFAL_WORKER_THREAD_PRIORITY), 0, K_NO_WAIT);

	VerifyOrReturnStatus(thread, ALIRO_INVALID_STATE, LOG_ERR("RFAL: Cannot spawn the NFC driver thread"));

	if (RfalNfcInit()) {
		LOG_ERR("RFAL: Init failed");
		return ALIRO_ERROR_INTERNAL;
	}

	k_timer_init(
		&mRxTimer,
		[](k_timer *) {
			LOG_DBG("RFAL: RX timer expired");
			Instance().mTimeout = true;
			Instance().DeselectTag();
		},
		nullptr);

	k_timer_init(
		&mIdleTimer,
		[](k_timer *) {
			LOG_DBG("RFAL: Idle timer expired");
			Instance().DeselectTag();
		},
		nullptr);

	return ALIRO_NO_ERROR;
}

AliroError NfcTransportRfal::_Send(NfcTransport::Data data, uint32_t maximumFrameDelayTime)
{
	k_timer_stop(&mIdleTimer);

	LOG_HEXDUMP_DBG(data.mData, data.mLength, "RFAL: TX data:");

	// use RFAL_FWT_NONE as FWT because the driver with ISO-DEP enabled will ignore it anyway
	ReturnCode err = rfalNfcDataExchangeStart(data.mData, data.mLength, &mRxData, &mRcvLen, RFAL_FWT_NONE);
	if (!err) {
		k_timer_start(&mRxTimer, K_MSEC(sRxTimerTimeoutMs), K_NO_WAIT);
		return ALIRO_NO_ERROR;
	}

	return ALIRO_ERROR_INTERNAL;
}

AliroError NfcTransportRfal::_FieldOn()
{
	// The RF field is turned right after the STR25 boots
	// The only thing that must be done is to activate the reader
	// by starting tag discovery procedure.
	ReturnCode err = rfalNfcDiscover(&mNfcConfig);
	if (err) {
		LOG_ERR("RFAL: NFC discovery failed, return code: %d", err);
		return ALIRO_ERROR_INTERNAL;
	}

	return ALIRO_NO_ERROR;
}

AliroError NfcTransportRfal::_FieldOff()
{
	// RFAL handles this internally and knows when the field can be off
	return ALIRO_ERROR_NOT_IMPLEMENTED;
}

AliroError NfcTransportRfal::_TagDetect(DetectType)
{
	// Deactivate and return to wakeup mode
	return Instance().DeselectTag();
}

AliroError NfcTransportRfal::_StartAnticollision()
{
	// Anticollision not required for if we support only a single tag at a time
	return ALIRO_ERROR_NOT_IMPLEMENTED;
}

AliroError NfcTransportRfal::_RecoverPolling(uint32_t delayMs, DetectType, bool)
{
	// RFAL automatically recovers to the polling state
	return ALIRO_NO_ERROR;
}

/* Implementation of the generic NfcDriver instance getter. */
NfcDriver &NfcDriverInstance()
{
	return NfcTransportRfal::Instance();
}
/*
******************************************************************************
******************************************************************************
*/

} // namespace Aliro
