/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include "uwb.h"

#include <aliro_uwb_adapter/aliro_uwb_adapter.h>
#include <aliro_uwb_adapter/aliro_uwb_session.h>
#include <cherry/cherry.h>

#include <zephyr/kernel.h>
#include <zephyr/sys/slist.h>

#include <cstddef>
#include <memory>
#include <optional>

struct cherry_radar_event;
struct cherry_radar_session;

namespace Aliro::Uwb {

/**
 * @class UltraWideBandImpl
 * @brief Implementation of the UltraWideBand interface.
 *
 * This class provides the implementation of the UltraWideBand interface, handling UWB operations
 * such as initialization, ranging session management, and BLE message handling.
 */
class UltraWideBandImpl final : public UltraWideBand {
	/**
	 * @brief Type alias for Cherry core event.
	 */
	using CoreEvent = cherry_core_event;

	/**
	 * @brief Type alias for Cherry event handler.
	 */
	using EventHandler = int (*)(CoreEvent *);

	/**
	 * @brief Type alias for active sessions list.
	 */
	using ActiveSessionsList = sys_slist_t;

	/**
	 * @brief Type alias for UWB session context.
	 */
	using UwbSessionContext = aliro_uwb_session *;

	/**
	 * @brief Helper enum class for UWB events.
	 */
	enum UwbEvents : uint32_t { Timeout = 0x00, Error = 0x01, DeviceCaps = 0x02, DeviceInfo = 0x04, All = 0xFFFF };

	/**
	 * @struct CccCaps
	 * @brief Struct containing CCC capabilities.
	 *
	 * This struct holds the CCC capabilities of the UWB module.
	 */
	struct CccCaps {
		uint8_t mSlotBitmask;
		uint8_t mChannelBitmask;
		uint8_t mHoppingConfigBitmask;
		uint32_t mSyncCodeIndexBitmask;
		uint8_t mMinimumRanMultiplier;
	};

	/**
	 * @brief Grants the UltraWideBand facade and singleton accessors access to private members.
	 *
	 * Only these friends may invoke the implementation methods and retrieve the singleton instance.
	 */
	friend class UltraWideBand;
	friend UltraWideBand &UltraWideBandInstance();
	friend UltraWideBandImpl &UltraWideBandInstanceImpl();

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

	/**
	 * @brief Implementation of the UltraWideBand interface.
	 */
	int _Init(const Callbacks &callbacks);
	int _Deinit();
	void _BleTimeSync();
	int _HandleBleMessage(const uint8_t *data, size_t length, SessionContextHandle sessionContextData);
	int _ConfigureRangingSession(SessionIdentifier sessionId, const CryptoTypes::Ursk &ursk,
				     ProtocolVersion protocolVersion, SessionContextHandle sessionContextHandle);
	int _InitiateRangingSession(SessionContextHandle sessionContextData);
	int _TerminateRangingSession(SessionContextHandle sessionContextData);
	int _SuspendRangingSession(SessionContextHandle sessionContextData);
	int _ResumeRangingSession(SessionContextHandle sessionContextData);
	const char *_GetFirmwareVersion() { return mQm35FirmwareVersion.get(); }
	bool _IsInitialized() { return mInitialized; }
	void _StopRadarSession();
#ifdef CONFIG_DOOR_LOCK_ALIRO_UWB_QM35_DISAMBIGUATION
	std::optional<uint8_t> _GetDisambiguationSessionIdx(SessionContextHandle sessionContextData);
#endif // CONFIG_DOOR_LOCK_ALIRO_UWB_QM35_DISAMBIGUATION

	UltraWideBandImpl() = default;
	~UltraWideBandImpl() = default;
	UltraWideBandImpl(const UltraWideBandImpl &) = delete;
	UltraWideBandImpl &operator=(const UltraWideBandImpl &) = delete;
	UltraWideBandImpl(UltraWideBandImpl &&) = delete;
	UltraWideBandImpl &operator=(UltraWideBandImpl &&) = delete;

	/**
	 * @brief Structure representing a UWB session context with list node.
	 *
	 * This structure contains the UWB session data and embeds a sys_snode_t
	 * for efficient linked list management following Zephyr patterns.
	 */
	struct SessionContext {
		SessionContext(UwbSessionContext uwbSessionContext, SessionContextHandle sessionContextData)
			: mUwbSessionContext(uwbSessionContext), mSessionContextData(sessionContextData)
		{
		}

		sys_snode_t mSessionContextNode{};
		UwbSessionContext mUwbSessionContext;
		SessionContextHandle mSessionContextData;
		cherry_ccc_session_state mSessionState{ CHERRY_CCC_SESSION_STATE_INIT };
		RangingSessionState mRangingSessionState{ RangingSessionState::Uninitialized };
#ifdef CONFIG_DOOR_LOCK_ALIRO_UWB_QM35_DISAMBIGUATION
		uint8_t mDisambiguationSessionIdx{ 0 };
#endif // CONFIG_DOOR_LOCK_ALIRO_UWB_QM35_DISAMBIGUATION
	};

	/**
	 * @brief Callback function for Cherry core events.
	 *
	 * This function handles core events from the Cherry framework and processes them accordingly.
	 *
	 * @param event Pointer to the Cherry core event.
	 * @param userData User data passed to the callback.
	 */
	static void UwbCoreCallback(cherry_core_event *event, void *userData);

	/**
	 * @brief Callback function to transmit BLE messages.
	 *
	 * This function is called to transmit BLE messages as part of the UWB session.
	 *
	 * @param message Pointer to the message to be transmitted.
	 * @param uwbSessionCtx Pointer to the UWB session context.
	 * @param userData User data passed to the callback.
	 * @param timeout Indicates if the transmission is a timeout operation
	 */
	static void TransmitBleMessage(aliro_uwb_message *message, UwbSessionContext uwbSessionCtx, void *userData,
				       bool timeout);

	/**
	 * @brief Callback function for session events.
	 *
	 * This function handles session events from the UWB session and processes them accordingly.
	 *
	 * @param event Pointer to the session event.
	 * @param userData User data passed to the callback.
	 */
	static void SessionHandlerCallback(aliro_uwb_session_event *event, void *userData);

	/**
	 * @brief Event handler for device capabilities event.
	 *
	 * @param event Pointer to the core event containing device capabilities data.
	 * @return 0 on success, negative error code otherwise.
	 */
	int HandleDeviceCapsEvent(CoreEvent *event);

	/**
	 * @brief Event handler for device info event.
	 *
	 * @param event Pointer to the core event containing device info data.
	 * @return 0 on success, negative error code otherwise.
	 */
	int HandleDeviceInfoEvent(CoreEvent *event);

	/**
	 * @brief Common helper function to wait for and handle UWB events.
	 *
	 * This function encapsulates the common pattern of waiting for a UWB event,
	 * checking for timeout/error conditions, and processing the event via a callback.
	 *
	 * @param expectedEvent The event type to wait for.
	 * @param eventHandler Function pointer to process the event data when received.
	 *
	 * @return 0 on success, negative error code otherwise.
	 */
	int HandleUwbEvent(UwbEvents expectedEvent, EventHandler eventHandler);

	/**
	 * @brief Finds a session context by session identifier.
	 *
	 * @param uwbSessionCtx The UWB session context.
	 *
	 * @return Pointer to SessionContext if found, nullptr otherwise.
	 */
	SessionContext *FindSession(const UwbSessionContext uwbSessionCtx);

	/**
	 * @brief Finds a session context by session context data.
	 *
	 * @param sessionContextData The session context data to search for.
	 *
	 * @return Pointer to SessionContext if found, nullptr otherwise.
	 */
	SessionContext *FindSession(const SessionContextHandle sessionContextData);

	/**
	 * @brief Adds a session context to the list.
	 *
	 * @param uwbSessionCtx The UWB session context.
	 * @param sessionContextHandle The session context data.
	 *
	 * @return 0 on success, negative error code otherwise.
	 */
	int AddSession(UwbSessionContext uwbSessionContext, SessionContextHandle sessionContextHandle);

	/**
	 * @brief Removes and destroys a session from the list.
	 *
	 * @param sessionCtx Pointer to the session context to remove.
	 */
	void RemoveSession(SessionContext *sessionCtx);

	/**
	 * @brief Removes all sessions from the list.
	 *
	 * This function iterates through the active sessions list and removes each session context,
	 * freeing associated resources.
	 */
	void RemoveAllSessions();

	/**
	 * @brief Destroys a UWB session.
	 *
	 * @param sessionCtx Pointer to the session context to destroy.
	 */
	void DestroySession(SessionContext *sessionCtx);

	void ScheduleRadarStart();
	void CancelRadarStart();
	int StartRadarSession();

	friend void RadarStartWorkHandler(k_work *work);

	void ScheduleDisambiguationTick();
	void CancelDisambiguationTick();
	friend void DisambiguationTickWorkHandler(k_work *work);
	static void RadarCallback(cherry_radar_event *event, void *userData);

	/**
	 * @brief Retrieves device information from the QM35 UWB device.
	 *
	 * @return 0 on success, negative error code otherwise.
	 */
	int GetDeviceInfo();

	/**
	 * @brief Retrieves device capabilities from the QM35 UWB device.
	 *
	 * @return 0 on success, negative error code otherwise.
	 */
	int GetDeviceCapabilities();

	/**
	 * @brief Prints the CCC capabilities of the QM35 UWB device.
	 */
	void PrintCccCapabilities();

	/**
	 * @brief Sets the calibration data for the QM35825 UWB device.
	 *
	 * @return 0 on success, negative error code otherwise.
	 */
	int SetCalibrationData();

	static constexpr size_t kCurrentDistanceBufferSize{ sizeof(
		cherry_ccc_session_controlee_measurements::distance_cm) };

	CoreEvent *mCoreEvent{};
	Callbacks mCallbacks{};
	cherry *mCtx{};
	aliro_uwb_adapter *mAliroCtx{};
	std::unique_ptr<char[]> mQm35FirmwareVersion{ nullptr };
	CccCaps mCccCaps{};
	bool mCccCapsValid{ false };
	bool mInitialized{ false };
	ActiveSessionsList mActiveSessionsList{};
	k_mutex mMutex{};
	std::array<uint8_t, kCurrentDistanceBufferSize> mCurrentDistanceCm{};

#ifdef CONFIG_DOOR_LOCK_ALIRO_UWB_QM35_DISAMBIGUATION
	std::array<SessionContext *, CONFIG_DOOR_LOCK_BLE_UWB_MAX_SESSIONS> mDisambiguationSessions{};
#endif // CONFIG_DOOR_LOCK_ALIRO_UWB_QM35_DISAMBIGUATION

	struct cherry_radar_session *mRadarSession{ nullptr };
	bool mRadarRunning{ false };
	bool mDisambiguationTickEnabled{ false };

	aliro_uwb_adapter_reader_config mReaderConfig = {
		.min_ran_multiplier = CONFIG_DOOR_LOCK_ALIRO_UWB_MIN_RAN_MULTIPLIER,
		.preferred_hopping_configs = { .configs = { ALIRO_HOPPING_CONFIG_DISABLED,
							    ALIRO_HOPPING_CONFIG_CONTINUOUS_DEFAULT },
					       .count = 2 },
		.mac_mode = static_cast<uint8_t>(CONFIG_DOOR_LOCK_ALIRO_UWB_MAC_MODE_OFFSET |
						 (CONFIG_DOOR_LOCK_ALIRO_UWB_MAC_MODE_RANGING_ROUNDS << 6)),
	};
};

/**
 * @brief Get the singleton instance of the UltraWideBand interface.
 *
 * @return Reference to the UltraWideBand instance.
 */
inline UltraWideBand &UltraWideBandInstance()
{
	return UltraWideBandImpl::Instance();
}

/**
 * @brief Get the singleton instance of the UltraWideBand implementation.
 *
 * @return Reference to the UltraWideBandImpl instance.
 */
inline UltraWideBandImpl &UltraWideBandInstanceImpl()
{
	return UltraWideBandImpl::Instance();
}

} // namespace Aliro::Uwb
