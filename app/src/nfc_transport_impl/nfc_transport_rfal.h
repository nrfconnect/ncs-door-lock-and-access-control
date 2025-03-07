/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include "transport_nfc/driver/interface/aliro_nfc_driver.h"
#include "transport_nfc/isodep/interface/aliro_isodep.h"

extern "C" {
#include <rfal_nfc.h>
#include <rfal_platform.h>
#include <rfal_utils.h>
}

#include <zephyr/kernel.h>

#include <array>

namespace Aliro {

class NfcTransportRfal : public IsoDep, public NfcDriver {
private:
	friend class IsoDep;
	friend IsoDep &IsoDepInstance();
	friend class NfcDriver;
	friend NfcDriver &NfcDriverInstance();

	// IsoDep interface
	AliroError _Init(IsoDep::Callbacks callbacks);
	AliroError _PrepareData(NfcTransport::Data data);
	AliroError _PrepareRats();
	AliroError _HandleReceivedData(NfcTransport::Data data, int transferError);
	AliroError _ReportTimeout();

	// NfcDriver interface
	AliroError _Init(NfcDriver::Callbacks callbacks);
	AliroError _Send(NfcTransport::Data data, uint32_t maximumFrameDelayTime);
	AliroError _NfcOn();
	AliroError _NfcOff();

	static NfcTransportRfal &Instance()
	{
		static NfcTransportRfal sInstance;
		return sInstance;
	}

	ReturnCode RfalNfcInit();
	void Run();
	void RfalNotifyCallback(rfalNfcState state);
	void CaptureRxData();
	void SelectTag();
	void RecoverPolling();

	rfalNfcDiscoverParam mNfcConfig{};
	k_thread mThread{};
	bool mMultiSel{ false };

	std::array<NfcTransport::Byte, CONFIG_RFAL_FEATURE_ISO_DEP_APDU_MAX_LEN> mRxBuffer{};
	uint8_t *mRxData{};
	uint16_t *mRcvLen{};

	k_timer mRxTimer{};
	k_timer mIdleTimer{};
	bool mRxTimeout{ false };
	bool mIdleTimeout{ false };

	static constexpr uint32_t sRxTimerTimeoutMs{ CONFIG_RFAL_RX_TIMEOUT_MS };
	static constexpr uint32_t sIdleTimerTimeoutMs{ CONFIG_RFAL_IDLE_TIMEOUT_MS };
};

} // namespace Aliro
