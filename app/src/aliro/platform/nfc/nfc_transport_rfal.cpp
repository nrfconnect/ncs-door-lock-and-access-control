/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "nfc_transport_rfal.h"
#include <rfal_ncs_pal.h>
#include <rfal_nfc_config.h>

#include "aliro/aliro.h"
#include "aliro/aliro_work/aliro_work.h"
#include "aliro/utils.h"
#include "ncs_pal_nfc_worker.h"

#ifdef CONFIG_DOOR_LOCK_NFC_PROP
#include "nfc_transport_rfal_prop.h"
#endif // CONFIG_DOOR_LOCK_NFC_PROP

#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(nfc_st_rfal_impl, CONFIG_DOOR_LOCK_RFAL_LOG_LEVEL);

K_WORK_DELAYABLE_DEFINE(nfc_pal_nfc_work, [](k_work *) { Aliro::NfcTransportRfal::Instance().Execute(); });

extern "C" void ncs_pal_submit_nfc_work()
{
	(void)AliroWorkReschedule(&nfc_pal_nfc_work, K_NO_WAIT);
}
namespace Aliro {

void NfcTransportRfal::Execute()
{
	// NFC shield generates IRQ on startup, triggering Execute() before RFAL is initialized.
	// Without this check, rfalNfcGetState() returns garbage which accidentally satisfies the
	// condition below, causing unnecessary periodic worker scheduling at CONFIG_RFAL_NFC_WORKER_INTERVAL_MS.
	if (!atomic_get(&mStarted)) {
		return;
	}

	if (mRecoverPolling && !mSendInProgress) {
		mRecoverPolling = false;
		RecoverPolling();
	}
	rfalNfcWorker();

	const rfalNfcState st = rfalNfcGetState();

	if (st != RFAL_NFC_STATE_WAKEUP_MODE || mSendInProgress) {
		(void)AliroWorkReschedule(&nfc_pal_nfc_work, K_MSEC(CONFIG_RFAL_NFC_WORKER_INTERVAL_MS));
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
		mTagDetectedState = false;
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
		mTagDetectedState = false;
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

void NfcTransportRfal::SelectTag()
{
	rfalNfcDevice *nfcDevice;
	rfalNfcGetActiveDevice(&nfcDevice);
	VerifyOrReturn(nfcDevice);

	LOG_INF("RFAL: Active device type = %d", static_cast<int>(nfcDevice->type));

	if (nfcDevice->type == RFAL_NFC_LISTEN_TYPE_NFCA) {
		if (nfcDevice->dev.nfca.type == RFAL_NFCA_T4T) {
			LOG_HEXDUMP_DBG(nfcDevice->nfcid, nfcDevice->nfcidLen,
					"RFAL: NFCA Passive ISO-DEP device found. UID: ");
			mTagDetectedState = true;
			AliroStack::Instance().CreateSession(ConnectionHandle::Nfc());
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
		mTagDetectedState = false;
		AliroStack::Instance().DestroySession(ConnectionHandle::Nfc());
		return;
	}

	uint16_t currentDataLen = *mRcvLen;
	memset(mRxBuffer.data(), 0, mRxBuffer.size());

	VerifyOrReturn(mRxBuffer.size() >= currentDataLen);
	memcpy(mRxBuffer.data(), mRxData, currentDataLen);

	LOG_HEXDUMP_DBG(mRxBuffer.data(), currentDataLen, "RFAL: RX data:");

	if (currentDataLen > 0) {
		AliroStack::Instance().HandleSessionData(ConnectionHandle::Nfc(),
							 { .mData = mRxBuffer.data(), .mLength = currentDataLen });
	} else {
		mTagDetectedState = false;
		AliroStack::Instance().DestroySession(ConnectionHandle::Nfc());
	}
}

void NfcTransportRfal::RecoverPolling()
{
	mTagDetectedState = false;
	if (rfalNfcIsDevActivated(rfalNfcGetState())) {
		ReturnCode err = rfalNfcDeactivate(RFAL_NFC_DEACTIVATE_SLEEP);
		VerifyOrReturn(err == RFAL_ERR_NONE, LOG_ERR("RFAL: Deactivation failed, return code: %d", err));
	}
}

/*
******************************************************************************
* Public API
******************************************************************************
*/

AliroError NfcTransportRfal::Init()
{
	int err = rfal_ncs_pal_init();
	VerifyOrReturnStatus(err == 0, ALIRO_ERROR_INTERNAL, LOG_ERR("RFAL: NFC PAL init failed %d", err));

	return ALIRO_NO_ERROR;
}

AliroError NfcTransportRfal::Start()
{
	VerifyOrReturnStatus(RfalNfcInit() == RFAL_ERR_NONE, ALIRO_ERROR_INTERNAL,
			     LOG_ERR("RFAL: NFC initialization failed"));

	ReturnCode err = rfalNfcDiscover(&mNfcConfig);
	VerifyOrReturnStatus(err == RFAL_ERR_NONE, ALIRO_ERROR_INTERNAL,
			     LOG_ERR("RFAL: NFC discovery failed, return code: %d", err));

	atomic_set(&mStarted, true);

	// Kickstart worker - at boot IRQ does this, but when starting with delay(provisioning) we need manual trigger
	ncs_pal_submit_nfc_work();

	return ALIRO_NO_ERROR;
}

AliroError NfcTransportRfal::Stop()
{
	ReturnCode err = rfalNfcDeactivate(RFAL_NFC_DEACTIVATE_IDLE);
	VerifyOrReturnStatus(err == RFAL_ERR_NONE || err == RFAL_ERR_WRONG_STATE, ALIRO_ERROR_INTERNAL,
			     LOG_ERR("RFAL: NFC deactivation failed, return code: %d", err));

	atomic_clear(&mStarted);
	return ALIRO_NO_ERROR;
}

AliroError NfcTransportRfal::Send(Data data)
{
	VerifyOrReturnStatus(mTagDetectedState, ALIRO_INVALID_STATE,
			     LOG_WRN("NFC not activated, no data transfer possible"));

	LOG_HEXDUMP_DBG(data.mData, data.mLength, "RFAL: TX data:");
	mSendInProgress = true;

	// use RFAL_FWT_NONE as FWT because the driver with ISO-DEP enabled will ignore it anyway
	ReturnCode err = rfalNfcDataExchangeStart(data.mData, data.mLength, &mRxData, &mRcvLen, RFAL_FWT_NONE);
	VerifyOrReturnStatus(err == RFAL_ERR_NONE, ALIRO_ERROR_INTERNAL,
			     LOG_ERR("RFAL: Data exchange failed, return code: %d", err);
			     mSendInProgress = false);

	return ALIRO_NO_ERROR;
}

AliroError NfcTransportRfal::Terminate()
{
	mRecoverPolling = true;
	return ALIRO_NO_ERROR;
}

} // namespace Aliro
