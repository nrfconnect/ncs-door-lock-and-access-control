/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include "aliro/errors.h"
#include "uwb/uwb.h"

#include <aliro_uwb_adapter/aliro_uwb_adapter.h>
#include <aliro_uwb_adapter/aliro_uwb_session.h>
#include <cherry/cherry.h>

#include <cstddef>

namespace Aliro::Uwb {

/**
 * @class UltraWideBandImpl
 * @brief Implementation of the UltraWideBand interface.
 *
 * This class provides the implementation of the UltraWideBand interface, handling UWB operations
 * such as initialization, ranging session management, and BLE message handling.
 */
class UltraWideBandImpl : public UltraWideBand<UltraWideBandImpl> {
	using CoreEvent = cherry_core_event;

public:
	/**
	 * @brief Gets the instance of the UltraWideBand implementation.
	 *
	 * @return The instance of the UltraWideBand implementation.
	 */
	static UltraWideBandImpl &Instance()
	{
		static UltraWideBandImpl sInstance;
		return sInstance;
	}

	AliroError _Init(const Callbacks &callbacks);
	AliroError _Deinit();
	void _BleTimeSync() const;
	AliroError _HandleBleMessage(const uint8_t *data, size_t length);
	AliroError _ConfigureRangingSession(SessionIdentifier sessionId, const CryptoTypes::Ursk &ursk,
					    void *sessionUserData);
	AliroError _InitiateRangingSession();
	AliroError _TerminateRangingSession();
	AliroError _SuspendRangingSession();
	AliroError _ResumeRangingSession();

	// Delete copy and move constructors and assignment operators.
	UltraWideBandImpl(const UltraWideBandImpl &) = delete;
	UltraWideBandImpl &operator=(const UltraWideBandImpl &) = delete;
	UltraWideBandImpl(UltraWideBandImpl &&) = delete;
	UltraWideBandImpl &operator=(UltraWideBandImpl &&) = delete;

	friend void UwbCoreCallback(cherry_core_event *event, void *userData);
	friend void TransmitBleMessage(aliro_uwb_message *message, aliro_uwb_session *sessionCtx, void *userData,
				       bool timeout);

protected:
	CoreEvent *mCoreEvent{};

private:
	static constexpr size_t kCurrentDistanceBufferSize{ sizeof(
		cherry_ccc_session_controlee_measurements::distance_cm) };

	UltraWideBandImpl() = default;
	~UltraWideBandImpl() = default;

	void SessionHandlerCallback(aliro_uwb_session_event *event, void *user_data);

	Callbacks mCallbacks{};
	cherry *mCtx{};
	aliro_uwb_adapter *mAliroCtx{};
	aliro_uwb_session *mAliroSessionCtx{};
	void *mAliroSessionUserData{};
	std::array<uint8_t, kCurrentDistanceBufferSize> mCurrentDistanceCm{};
	aliro_uwb_adapter_reader_config mReaderConfig = {
		.min_ran_multiplier = CONFIG_ALIRO_UWB_MIN_RAN_MULTIPLIER,
		.preferred_hopping_config = { CONFIG_ALIRO_UWB_PREFERRED_HOPPING_CONFIG_VALUE, 0x00, 0x00, 0x00, 0x00 },
		.mac_mode =
			(uint8_t)(CONFIG_ALIRO_UWB_MAC_MODE_OFFSET | (CONFIG_ALIRO_UWB_MAC_MODE_RANGING_ROUNDS << 6)),
	};
};

} // namespace Aliro::Uwb
