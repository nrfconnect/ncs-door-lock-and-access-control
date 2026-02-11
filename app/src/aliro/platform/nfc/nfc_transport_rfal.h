/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include "aliro/errors.h"
#include "aliro/types.h"

extern "C" {
#include <rfal_nfc.h>
#include <rfal_platform.h>
#include <rfal_utils.h>
}

#include <zephyr/kernel.h>

#include <array>

namespace Aliro {

/**
 * @class NfcTransportRfal
 * @brief NFC transport implementation using ST RFAL library.
 *
 */
class NfcTransportRfal {
public:
	/**
	 * @brief Gets the singleton instance.
	 * @return Reference to the singleton instance.
	 */
	static NfcTransportRfal &Instance()
	{
		static NfcTransportRfal sInstance;
		return sInstance;
	}

	/**
	 * @brief Initializes the NFC transport.
	 * @return ALIRO_NO_ERROR on success, error code otherwise.
	 */
	AliroError Init();

	/**
	 * @brief Starts NFC polling/discovery.
	 * @return ALIRO_NO_ERROR on success, error code otherwise.
	 */
	AliroError Start();

	/**
	 * @brief Stops NFC polling.
	 * @return ALIRO_NO_ERROR on success, error code otherwise.
	 */
	AliroError Stop();

	/**
	 * @brief Sends data to the detected NFC card.
	 *
	 * @param data The data to send.
	 * @return ALIRO_NO_ERROR on success, error code otherwise.
	 */
	AliroError Send(Data data);

	/**
	 * @brief Terminates the current NFC session and restarts polling.
	 *
	 * @return ALIRO_NO_ERROR on success, error code otherwise.
	 */
	AliroError Terminate();

private:
	NfcTransportRfal() = default;
	NfcTransportRfal(const NfcTransportRfal &) = delete;
	NfcTransportRfal(NfcTransportRfal &&) = delete;
	~NfcTransportRfal() = default;
	NfcTransportRfal &operator=(const NfcTransportRfal &) = delete;
	NfcTransportRfal &operator=(NfcTransportRfal &&) = delete;

	ReturnCode RfalNfcInit();
	[[noreturn]] void Run();
	void RfalNotifyCallback(rfalNfcState state);
	void CaptureRxData();
	void SelectTag();
	void RecoverPolling();

	rfalNfcDiscoverParam mNfcConfig{};
	k_thread mThread{};
	bool mMultiSel{ false };

	std::array<uint8_t, CONFIG_RFAL_FEATURE_ISO_DEP_APDU_MAX_LEN> mRxBuffer{};
	uint8_t *mRxData{};
	uint16_t *mRcvLen{};

	bool mRecoverPolling{ false };
	bool mSendInProgress{ false };
	bool mTagDetectedState{ false };
};

} // namespace Aliro
