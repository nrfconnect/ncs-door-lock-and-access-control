/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "nfc_transport_rfal.h"
#include "ncs_pal_semaphore.h"
#include <rfal_ncs_pal.h>
#include <rfal_nfc_config.h>

#include "aliro/utils.h"
#include "ncs_pal_nfc_worker.h"

#ifdef CONFIG_DOOR_LOCK_NFC_PROP
#include "nfc_transport_rfal_prop.h"
#endif // CONFIG_DOOR_LOCK_NFC_PROP

#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(nfc_st_rfal_impl, CONFIG_DOOR_LOCK_RFAL_LOG_LEVEL);

namespace Aliro {

K_THREAD_STACK_DEFINE(mStack, CONFIG_RFAL_WORKER_THREAD_STACK_SIZE);

[[noreturn]] void NfcTransportRfal::Run()
{
	while (true) {
		if (mRecoverPolling && !mSendInProgress) {
			mRecoverPolling = false;
			RecoverPolling();
		}
		rfalNfcWorker();
		ncs_pal_take_semaphore(K_MSEC(CONFIG_RFAL_NFC_WORKER_TIMEOUT_MS));
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
			(void)rfalNfcGetDevicesFound(&dev, &devCnt);
			(void)rfalNfcSelect(0);
			LOG_DBG("RFAL: Multiple Tags detected: %d", devCnt);
		} else {
			(void)rfalNfcDeactivate(RFAL_NFC_DEACTIVATE_DISCOVERY);
		}
		break;
	case RFAL_NFC_STATE_START_DISCOVERY:
		LOG_DBG("RFAL: Start discovery state");
		mMultiSel = false;
		mSendInProgress = false;
		break;
	case RFAL_NFC_STATE_DATAEXCHANGE:
		LOG_DBG("RFAL: Data exchange state");
		break;
	case RFAL_NFC_STATE_DATAEXCHANGE_DONE:
		mSendInProgress = false;
		CaptureRxData();
		break;
	case RFAL_NFC_STATE_DEACTIVATION:
		LOG_DBG("RFAL: Deactivation State");
		mSendInProgress = false;
		break;
	case RFAL_NFC_STATE_ACTIVATED:
		LOG_DBG("RFAL: Activated state");
		SelectTag();
		break;
	default:
		break;
	}
}

ReturnCode NfcTransportRfal::RfalNfcInit()
{
	ReturnCode err = rfalNfcInitialize();
	VerifyOrExit(err == RFAL_ERR_NONE);

	rfalNfcDefaultDiscParams(&mNfcConfig);

	rfalNfcWakeupConfig(&mNfcConfig);

	mNfcConfig.devLimit = CONFIG_RFAL_DISCOVERY_DEV_LIMIT;
	mNfcConfig.totalDuration = CONFIG_RFAL_DISCOVERY_TOTAL_DURATION_MS;
	mNfcConfig.GBLen = CONFIG_RFAL_DISCOVERY_GBLEN;
	mNfcConfig.maxBR = static_cast<rfalBitRate>(CONFIG_RFAL_DISCOVERY_MAX_BR);

#if defined(CONFIG_RFAL_DISCOVERY_COMP_MODE_EMV)
	mNfcConfig.compMode = RFAL_COMPLIANCE_MODE_EMV;
#elif defined(CONFIG_RFAL_DISCOVERY_COMP_MODE_ISO)
	mNfcConfig.compMode = RFAL_COMPLIANCE_MODE_ISO;
#endif

	mNfcConfig.techs2Find = RFAL_NFC_POLL_TECH_A;

#ifdef CONFIG_DOOR_LOCK_NFC_PROP
	{
		mNfcConfig.techs2Find |= RFAL_NFC_POLL_TECH_PROP;

		const rfalNfcPropCallbacks *propCallbacks = NfcPropGetCallbacks();
		if (propCallbacks) {
			mNfcConfig.propNfc = *propCallbacks;
			NfcPropInit();
			LOG_DBG("RFAL: Proprietary technology registered");
		}
	}
#endif // CONFIG_DOOR_LOCK_NFC_PROP

	LOG_DBG("RFAL: Discovery configured (devLimit=%d, duration=%dms, maxBR=%d, compMode=%d)", mNfcConfig.devLimit,
		mNfcConfig.totalDuration, mNfcConfig.maxBR, mNfcConfig.compMode);

	mNfcConfig.notifyCb = [](rfalNfcState state) { Instance().RfalNotifyCallback(state); };
exit:
	return err;
}

void NfcTransportRfal::SelectTag() const
{
	rfalNfcDevice *nfcDevice;
	rfalNfcGetActiveDevice(&nfcDevice);
	VerifyOrReturn(nfcDevice);

	LOG_INF("RFAL: Active device type = %d", nfcDevice->type);

	if (nfcDevice->type == RFAL_NFC_LISTEN_TYPE_NFCA) {
		if (nfcDevice->dev.nfca.type == RFAL_NFCA_T4T) {
			LOG_HEXDUMP_DBG(nfcDevice->nfcid, nfcDevice->nfcidLen,
					"RFAL: NFCA Passive ISO-DEP device found. UID: ");
			VerifyAndCall(this->NfcDriver::mCallbacks.mOnTagDetected);
		} else {
			LOG_HEXDUMP_DBG(nfcDevice->nfcid, nfcDevice->nfcidLen,
					"RFAL: Usupported NFC card found. UID: %s");
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

void NfcTransportRfal::RecoverPolling() const
{
	if (rfalNfcIsDevActivated(rfalNfcGetState())) {
		ReturnCode err = rfalNfcDeactivate(RFAL_NFC_DEACTIVATE_SLEEP);
		VerifyOrReturn(err == RFAL_ERR_NONE, LOG_ERR("RFAL: Deactivation failed, return code: %d", err));
	}
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

AliroError NfcTransportRfal::_PrepareData([[maybe_unused]] Data data) const
{
	// ISO-DEP layer is implemented internally in the RFAL, no need for special data handling
	return ALIRO_ERROR_NOT_IMPLEMENTED;
}

AliroError NfcTransportRfal::_PrepareRats() const
{
	// RATS is sent by the driver internally as an activation procedure, so we can presume the tag is fully selected
	// a this point
	return ALIRO_ERROR_NOT_IMPLEMENTED;
}

AliroError NfcTransportRfal::_HandleReceivedData([[maybe_unused]] Data data, [[maybe_unused]] int transferError) const
{
	// No specific processing needed, all ISO-DEP specific data handling happens in driver's internals
	return ALIRO_ERROR_NOT_IMPLEMENTED;
}

AliroError NfcTransportRfal::_ReportTimeout() const
{
	// No special handling needed with RFAL
	return ALIRO_ERROR_NOT_IMPLEMENTED;
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
	VerifyOrReturnStatus(err == 0, ALIRO_ERROR_INTERNAL, LOG_ERR("RFAL: NFC PAL init failed %d", err));

	VerifyOrReturnStatus(RfalNfcInit() == RFAL_ERR_NONE, ALIRO_ERROR_INTERNAL,
			     LOG_ERR("RFAL: NFC initialization failed"));

	const k_tid_t thread = ncs_pal_nfc_worker_start([](void *, void *, void *) { Instance().Run(); });
	VerifyOrReturnStatus(thread, ALIRO_INVALID_STATE, LOG_ERR("RFAL: Cannot spawn the NFC driver thread"));

	return ALIRO_NO_ERROR;
}

AliroError NfcTransportRfal::_Send(Data data, [[maybe_unused]] uint32_t maximumFrameDelayTime)
{
	LOG_HEXDUMP_DBG(data.mData, data.mLength, "RFAL: TX data:");
	mSendInProgress = true;

	// use RFAL_FWT_NONE as FWT because the driver with ISO-DEP enabled will ignore it anyway
	ReturnCode err = rfalNfcDataExchangeStart(data.mData, data.mLength, &mRxData, &mRcvLen, RFAL_FWT_NONE);
	VerifyOrReturnStatus(err == RFAL_ERR_NONE, ALIRO_ERROR_INTERNAL,
			     LOG_ERR("RFAL: Data exchange failed, return code: %d", err);
			     mSendInProgress = false);

	return ALIRO_NO_ERROR;
}

AliroError NfcTransportRfal::_NfcOn() const
{
	VerifyOrReturnStatus(NfcTransportRfal::Instance().RfalNfcInit() == RFAL_ERR_NONE, ALIRO_ERROR_INTERNAL,
			     LOG_ERR("RFAL: NFC initialization failed"));

	ReturnCode err = rfalNfcDiscover(&mNfcConfig);
	VerifyOrReturnStatus(err == RFAL_ERR_NONE, ALIRO_ERROR_INTERNAL,
			     LOG_ERR("RFAL: NFC discovery failed, return code: %d", err));

	return ALIRO_NO_ERROR;
}

AliroError NfcTransportRfal::_NfcOff() const
{
	// RFAL handles this internally and knows when the field can be off
	return ALIRO_NO_ERROR;
}

AliroError NfcTransportRfal::_RestartPolling()
{
	mRecoverPolling = true;

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
