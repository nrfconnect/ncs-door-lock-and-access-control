/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include "transport/nfc/driver/interface/aliro_nfc_driver.h"
#include "transport/nfc/isodep/interface/aliro_isodep.h"

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
	AliroError _PrepareData(Data data) const;
	AliroError _PrepareRats() const;
	AliroError _HandleReceivedData(Data data, int transferError) const;
	AliroError _ReportTimeout() const;

	// NfcDriver interface
	AliroError _Init(NfcDriver::Callbacks callbacks);
	AliroError _Send(Data data, uint32_t maximumFrameDelayTime);
	AliroError _NfcOn() const;
	AliroError _NfcOff() const;

	static NfcTransportRfal &Instance()
	{
		static NfcTransportRfal sInstance;
		return sInstance;
	}

	ReturnCode RfalNfcInit();
	void Run();
	void RfalNotifyCallback(rfalNfcState state);
	void CaptureRxData();
	void SelectTag() const;
	void RecoverPolling() const;

	rfalNfcDiscoverParam mNfcConfig{};
	k_thread mThread{};
	bool mMultiSel{ false };

	std::array<Byte, CONFIG_RFAL_FEATURE_ISO_DEP_APDU_MAX_LEN> mRxBuffer{};
	uint8_t *mRxData{};
	uint16_t *mRcvLen{};

	k_timer mIdleTimer{};
	bool mIdleTimeout{ false };

	static constexpr uint32_t sIdleTimerTimeoutMs{ CONFIG_RFAL_IDLE_TIMEOUT_MS };
};

} // namespace Aliro
