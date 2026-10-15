/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <nfc_transport/nfc_transport_backend.h>

extern "C" {
#include <rfal_nfc.h>
#include <rfal_platform.h>
#include <rfal_utils.h>
}
#include <rfal_ncs_pal.h>
#include <rfal_nfc_config.h>

#include "ncs_pal_nfc_worker.h"

#include <aliro/utils.h>
#include <aliro_workqueue/aliro_workqueue.h>

#ifdef CONFIG_DOOR_LOCK_NFC_PROP
#include <nfc_prop/nfc_prop.h>
#endif // CONFIG_DOOR_LOCK_NFC_PROP

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/atomic.h>

#include <array>

LOG_MODULE_REGISTER(aliro_nfc_transport, CONFIG_DOOR_LOCK_ALIRO_NFC_TRANSPORT_LOG_LEVEL);

namespace Aliro::NfcTransport::Backend {
namespace {

/**
 * @class NfcTransportRfal
 * @brief NFC transport backend using ST RFAL library.
 */
class NfcTransportRfal {
public:
	static NfcTransportRfal &Instance()
	{
		static NfcTransportRfal sInstance;
		return sInstance;
	}

	AliroError Init(const Callbacks &callbacks);
	AliroError Start();
	AliroError Stop();
	AliroError Send(Data data);
	AliroError Terminate();

	/**
	 * @brief Execute a single RFAL NFC worker iteration.
	 */
	void Execute();

private:
	NfcTransportRfal() = default;
	~NfcTransportRfal() = default;

	NfcTransportRfal(const NfcTransportRfal &) = delete;
	NfcTransportRfal &operator=(const NfcTransportRfal &) = delete;
	NfcTransportRfal(NfcTransportRfal &&) = delete;
	NfcTransportRfal &operator=(NfcTransportRfal &&) = delete;

	ReturnCode RfalNfcInit();
	void RfalNotifyCallback(rfalNfcState state);
	void CaptureRxData();
	void SelectTag();
	void RecoverPolling();

	Callbacks mCallbacks{};
	rfalNfcDiscoverParam mNfcConfig{};
	bool mMultiSel{ false };

	std::array<uint8_t, CONFIG_RFAL_FEATURE_ISO_DEP_APDU_MAX_LEN> mRxBuffer{};
	uint8_t *mRxData{};
	uint16_t *mRcvLen{};

	bool mRecoverPolling{ false };
	bool mSendInProgress{ false };
	bool mTagDetectedState{ false };
	atomic_t mStarted{ false };
};

} // namespace
} // namespace Aliro::NfcTransport::Backend

using Aliro::NfcTransport::Backend::NfcTransportRfal;

K_WORK_DELAYABLE_DEFINE(nfc_pal_nfc_work, [](k_work *) { NfcTransportRfal::Instance().Execute(); });

extern "C" void ncs_pal_submit_nfc_work()
{
	(void)AliroWorkqueueReschedule(&nfc_pal_nfc_work, K_NO_WAIT);
}

namespace Aliro::NfcTransport::Backend {
namespace {

void NfcTransportRfal::Execute()
{
	// NFC shield generates IRQ on startup, triggering Execute() before RFAL is initialized.
	// Without this check, rfalNfcGetState() returns garbage which accidentally satisfies the
	// condition below, causing unnecessary periodic worker scheduling at
	// CONFIG_DOOR_LOCK_ALIRO_NFC_TRANSPORT_RFAL_WORKER_INTERVAL_MS.
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
		(void)AliroWorkqueueReschedule(&nfc_pal_nfc_work,
					       K_MSEC(CONFIG_DOOR_LOCK_ALIRO_NFC_TRANSPORT_RFAL_WORKER_INTERVAL_MS));
	}
}

void NfcTransportRfal::RfalNotifyCallback(rfalNfcState state)
{
	switch (state) {
	case RFAL_NFC_STATE_WAKEUP_MODE:
		break;
	case RFAL_NFC_STATE_POLL_TECHDETECT:
		break;
	case RFAL_NFC_STATE_POLL_SELECT:
		// Check if in case of multiple devices, selection is already attempted
		if (!mMultiSel) {
			mMultiSel = true;
			// Multiple devices were found, activate first of them
			uint8_t devCnt;
			rfalNfcDevice *dev;
			(void)rfalNfcGetDevicesFound(&dev, &devCnt);
			(void)rfalNfcSelect(0);
		} else {
			(void)rfalNfcDeactivate(RFAL_NFC_DEACTIVATE_DISCOVERY);
		}
		break;
	case RFAL_NFC_STATE_START_DISCOVERY:
		mMultiSel = false;
		mSendInProgress = false;
		mTagDetectedState = false;
		break;
	case RFAL_NFC_STATE_DATAEXCHANGE:
		break;
	case RFAL_NFC_STATE_DATAEXCHANGE_DONE:
		mSendInProgress = false;
		CaptureRxData();
		break;
	case RFAL_NFC_STATE_DEACTIVATION:
		mSendInProgress = false;
		mTagDetectedState = false;
		break;
	case RFAL_NFC_STATE_ACTIVATED:
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

	mNfcConfig.devLimit = CONFIG_DOOR_LOCK_ALIRO_NFC_TRANSPORT_RFAL_DISCOVERY_DEV_LIMIT;
	mNfcConfig.totalDuration = CONFIG_DOOR_LOCK_ALIRO_NFC_TRANSPORT_RFAL_DISCOVERY_TOTAL_DURATION_MS;
	mNfcConfig.GBLen = CONFIG_DOOR_LOCK_ALIRO_NFC_TRANSPORT_RFAL_DISCOVERY_GBLEN;
	mNfcConfig.maxBR = static_cast<rfalBitRate>(CONFIG_DOOR_LOCK_ALIRO_NFC_TRANSPORT_RFAL_DISCOVERY_MAX_BR);

#if defined(CONFIG_DOOR_LOCK_ALIRO_NFC_TRANSPORT_RFAL_DISCOVERY_COMP_MODE_EMV)
	mNfcConfig.compMode = RFAL_COMPLIANCE_MODE_EMV;
#elif defined(CONFIG_DOOR_LOCK_ALIRO_NFC_TRANSPORT_RFAL_DISCOVERY_COMP_MODE_ISO)
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
			const AliroError err = mCallbacks.mOnDeviceDetected();
			if (err != ALIRO_NO_ERROR) {
				LOG_ERR("RFAL: Device rejected: %d", err.ToInt());
				mTagDetectedState = false;
				mRecoverPolling = true;
			}
		} else {
			LOG_HEXDUMP_DBG(nfcDevice->nfcid, nfcDevice->nfcidLen,
					"RFAL: Usupported NFC card found. UID: %s");
		}
	}
}

void NfcTransportRfal::CaptureRxData()
{
	ReturnCode status = rfalNfcDataExchangeGetStatus();
	if (status != RFAL_ERR_NONE) {
		LOG_ERR("RFAL: Data transaction failed [status: %d]", status);
		mTagDetectedState = false;
		mCallbacks.mOnDeviceLost();
		return;
	}

	uint16_t currentDataLen = *mRcvLen;
	memset(mRxBuffer.data(), 0, mRxBuffer.size());

	VerifyOrReturn(mRxBuffer.size() >= currentDataLen);
	memcpy(mRxBuffer.data(), mRxData, currentDataLen);

	LOG_HEXDUMP_DBG(mRxBuffer.data(), currentDataLen, "RFAL: RX data:");

	if (currentDataLen > 0) {
		mCallbacks.mOnDataReceived({ .mData = mRxBuffer.data(), .mLength = currentDataLen });
	} else {
		mTagDetectedState = false;
		mCallbacks.mOnDeviceLost();
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
* NfcTransportRfal API
******************************************************************************
*/

AliroError NfcTransportRfal::Init(const Callbacks &callbacks)
{
	VerifyOrReturnStatus(callbacks.mOnDeviceDetected && callbacks.mOnDataReceived && callbacks.mOnDeviceLost,
			     ALIRO_INVALID_ARGUMENT);
	mCallbacks = callbacks;

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

} // namespace

AliroError Init(const Callbacks &callbacks)
{
	return NfcTransportRfal::Instance().Init(callbacks);
}

AliroError Start()
{
	return NfcTransportRfal::Instance().Start();
}

AliroError Stop()
{
	return NfcTransportRfal::Instance().Stop();
}

AliroError Send(Data data)
{
	return NfcTransportRfal::Instance().Send(data);
}

AliroError Terminate()
{
	return NfcTransportRfal::Instance().Terminate();
}

} // namespace Aliro::NfcTransport::Backend
