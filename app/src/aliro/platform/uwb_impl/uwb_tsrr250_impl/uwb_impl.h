/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include "aliro/errors.h"
#include "uwb.h"

#include <aliro_uwb_adapter/aliro_uwb_adapter.h>
#include <aliro_uwb_adapter/aliro_uwb_session.h>
#include <cherry/cherry.h>
#include <cherry/cherry_ccc.h>

#include <zephyr/kernel.h>
#include <zephyr/sys/util.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>

namespace Aliro::Uwb {

/**
 * @brief Aliro UWB backend for the TrueSense TSRR250 / NXP SR250 module.
 *
 * This class is the app-side integration point. The NXP UWB API calls from the
 * SR250 CSA controlee demo are intentionally kept behind this backend so the
 * rest of the Aliro stack does not depend on a specific UWB vendor API.
 */
class UltraWideBandImpl : public UltraWideBand<UltraWideBandImpl> {
public:
	static UltraWideBandImpl &Instance()
	{
		static UltraWideBandImpl sInstance;
		return sInstance;
	}

	AliroError _Init(const Callbacks &callbacks);
	AliroError _Deinit();
	void _BleTimeSync();
	AliroError _HandleBleMessage(const uint8_t *data, size_t length, SessionContextHandle sessionContextData);
	AliroError _ConfigureRangingSession(SessionIdentifier sessionId, const CryptoTypes::Ursk &ursk,
					    ProtocolVersion protocolVersion, SessionContextHandle sessionContextHandle);
	AliroError _InitiateRangingSession(SessionContextHandle sessionContextData);
	AliroError _TerminateRangingSession(SessionContextHandle sessionContextData);
	AliroError _SuspendRangingSession(SessionContextHandle sessionContextData, bool force);
	AliroError _ResumeRangingSession(SessionContextHandle sessionContextData);

	bool IsInitialized() const { return mInitialized; }
	const char *GetDeviceName() const { return "TSRR250/SR250"; }
	static void HandleNxpNotification(uint8_t notificationType, void *data, size_t length);

	UltraWideBandImpl(const UltraWideBandImpl &) = delete;
	UltraWideBandImpl &operator=(const UltraWideBandImpl &) = delete;
	UltraWideBandImpl(UltraWideBandImpl &&) = delete;
	UltraWideBandImpl &operator=(UltraWideBandImpl &&) = delete;

private:
	using UwbSessionContext = aliro_uwb_session *;

	static constexpr size_t kSessionIdLength{ 4 };
	static constexpr size_t kRandomKeyLength{ 12 };
	static constexpr size_t kCccSessionKeyLength{ CryptoTypes::kSymmetricKeyLength };
	static constexpr size_t kWrappedRdsLength{ kSessionIdLength + kRandomKeyLength + kCccSessionKeyLength };
	static constexpr size_t kCurrentDistanceBufferSize{ sizeof(uint16_t) };

	using WrappedRds = std::array<uint8_t, kWrappedRdsLength>;

	struct SessionContext {
		SessionContext(SessionIdentifier sessionId, const CryptoTypes::Ursk &ursk, ProtocolVersion protocolVersion,
			       SessionContextHandle sessionContextHandle);

		struct RssM2Selection {
			uint16_t mUwbConfigId{};
			uint8_t mPulseShapeCombo{};
			uint8_t mChannelBitmask{};
			uint32_t mSyncCodeIndexBitmask{};
			uint8_t mRanMultiplier{};
			uint8_t mSlotBitmask{};
			uint8_t mHoppingConfigBitmask{};
			bool mValid{};
		};

		SessionIdentifier mSessionId;
		CryptoTypes::Ursk mUrsk;
		ProtocolVersion mProtocolVersion;
		SessionContextHandle mSessionContextHandle;
		RangingSessionState mState{ RangingSessionState::Initialized };
		WrappedRds mWrappedRds{};
		UwbSessionContext mAliroSessionContext{};
		uint32_t mNxpSessionHandle{};
		uint32_t mLastStsIndex0{};
		RssM2Selection mRssM2{};
		bool mNxpSessionInitialized{};
		bool mRangingStarted{};
		bool mRssM1Sent{};
		bool mRssM3Sent{};
		bool mRssM4Received{};
		uint8_t mSyncCodeIndex{ 9 };
		std::array<uint16_t, 3> mCsaDistanceSamples{};
		uint8_t mCsaDistanceSampleCount{};
	};

	UltraWideBandImpl() = default;
	~UltraWideBandImpl() = default;

	static WrappedRds BuildWrappedRds(uint32_t nxpSessionHandle, const CryptoTypes::Ursk &ursk);
	static void TransmitBleMessage(aliro_uwb_message *message, UwbSessionContext uwbSessionCtx, void *userData,
				       bool timeout);
	static void SessionHandlerCallback(aliro_uwb_session_event *event, void *userData);
	SessionContext *FindSession(SessionContextHandle sessionContextHandle);
	SessionContext *FindSession(UwbSessionContext uwbSessionContext);
	AliroError CreateAliroAdapter();
	AliroError CreateAliroSession(SessionContext &session);
	AliroError HandleRssM2(SessionContext &session, const uint8_t *data, size_t length);
	AliroError HandleRssM4(SessionContext &session, const uint8_t *data, size_t length);
	AliroError HandleRangingSessionSuspendRequest(SessionContext &session, const uint8_t *data, size_t length);
	AliroError HandleRangingSessionResumeRequest(SessionContext &session, const uint8_t *data, size_t length);
	AliroError HandleRangingSessionResumeResponse(SessionContext &session, const uint8_t *data, size_t length);
	AliroError HandleRangingNotification(SessionContext &session, const uint8_t *data, size_t length);
	AliroError SendRssM1(SessionContext &session);
	AliroError SendRssM3(SessionContext &session);
	AliroError SendRssGeneralError(SessionContext &session, uint8_t reason);
	AliroError SendRangingSessionSuspendRequest(SessionContext &session);
	AliroError SendRangingSessionSuspendResponse(SessionContext &session, uint8_t status);
	AliroError SendRangingSessionResumeRequest(SessionContext &session);
		AliroError SendRangingSessionResumeResponse(SessionContext &session);
		AliroError ConfigureNxpSession(SessionContext &session);
		AliroError StartNxpRangingSession(SessionContext &session);
		AliroError StopNxpRangingSession(SessionContext &session);
		AliroError ApplySr250Calibration();
		void ResetCsaDistanceFilter(SessionContext &session);
		bool AddCsaDistanceSample(SessionContext &session, uint16_t distanceCm, uint16_t &filteredDistanceCm);
		void ReportRangingDistance(SessionContext &session, uint16_t distanceCm);
	void NotifySessionState(SessionContext &session, RangingSessionState state);

	Callbacks mCallbacks{};
	aliro_uwb_adapter *mAliroCtx{};
	std::unique_ptr<SessionContext> mSession{};
	bool mInitialized{};
	std::array<uint8_t, kCurrentDistanceBufferSize> mCurrentDistanceCm{};
	std::array<uint16_t, 1> mProtocolVersions{ 0x0100 };
	std::array<uint16_t, 2> mUwbConfigs{ 0x0000, 0x0001 };
	std::array<uint8_t, 3> mPulseShapeCombos{ 0x00, 0x11, 0x22 };
	cherry_ccc_capabilities mCccCaps{
		.slot_bitmask = BIT(3),
		.sync_code_index_bitmask = BIT(8),
		.hopping_config_bitmask = BIT(0) | BIT(1) | BIT(2) | BIT(3) | BIT(4),
		.channel_bitmask = BIT(0) | BIT(1),
		.protocol_versions = { .len = mProtocolVersions.size(), .items = mProtocolVersions.data() },
		.uwb_configs = { .len = mUwbConfigs.size(), .items = mUwbConfigs.data() },
		.pulse_shape_combos = { .len = mPulseShapeCombos.size(), .items = mPulseShapeCombos.data() },
		.minimum_ran_multiplier = CONFIG_DOOR_LOCK_UWB_MIN_RAN_MULTIPLIER,
	};
	cherry_core_event_device_capabilities mDeviceCaps{ .status_err = CHERRY_ERR_NONE,
							   .ccc_capabilities = &mCccCaps };
	aliro_uwb_adapter_reader_config mReaderConfig{
		.min_ran_multiplier = CONFIG_DOOR_LOCK_UWB_MIN_RAN_MULTIPLIER,
		.preferred_hopping_configs = { .configs = { ALIRO_HOPPING_CONFIG_DISABLED,
							    ALIRO_HOPPING_CONFIG_CONTINUOUS_DEFAULT },
					       .count = 2 },
		.mac_mode = static_cast<uint8_t>(CONFIG_DOOR_LOCK_UWB_MAC_MODE_OFFSET |
							 (CONFIG_DOOR_LOCK_UWB_MAC_MODE_RANGING_ROUNDS << 6)),
	};
};

} // namespace Aliro::Uwb
