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

#include <zephyr/kernel.h>
#include <zephyr/sys/slist.h>

#include <cstddef>
#include <memory>

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
	using EventHandler = AliroError (*)(CoreEvent *);

	/**
	 * @brief Helper enum class for UWB events.
	 */
	enum UwbEvents : uint32_t { Timeout = 0x00, Error = 0x01, DeviceCaps = 0x02, DeviceInfo = 0x04, All = 0xFFFF };

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
	void _BleTimeSync();
	AliroError _HandleBleMessage(const uint8_t *data, size_t length, SessionContextHandle sessionContextData);
	AliroError _ConfigureRangingSession(SessionIdentifier sessionId, const CryptoTypes::Ursk &ursk,
					    SessionContextHandle sessionContextData);
	AliroError _InitiateRangingSession(SessionContextHandle sessionContextData);
	AliroError _TerminateRangingSession(SessionContextHandle sessionContextData);
	AliroError _SuspendRangingSession(SessionContextHandle sessionContextData);
	AliroError _ResumeRangingSession(SessionContextHandle sessionContextData);

	/**
	 * @brief Gets the QM35 firmware version string.
	 *
	 * @return Pointer to the firmware version string, or nullptr if not available.
	 */
	const char *GetQm35FirmwareVersion() const { return mQm35FirmwareVersion.get(); }

	// Delete copy and move constructors and assignment operators.
	UltraWideBandImpl(const UltraWideBandImpl &) = delete;
	UltraWideBandImpl &operator=(const UltraWideBandImpl &) = delete;
	UltraWideBandImpl(UltraWideBandImpl &&) = delete;
	UltraWideBandImpl &operator=(UltraWideBandImpl &&) = delete;

private:
	using ActiveSessionsList = sys_slist_t;
	using UwbSessionContext = aliro_uwb_session *;

	/**
	 * @brief Structure representing a UWB session context with list node.
	 *
	 * This structure contains the UWB session data and embeds a sys_snode_t
	 * for efficient linked list management following Zephyr patterns.
	 */
	struct SessionContext {
		sys_snode_t mSessionContextNode{};
		UwbSessionContext mUwbSessionContext{};
		SessionContextHandle mSessionContextData{};
		cherry_ccc_session_state mSessionState{ CHERRY_CCC_SESSION_STATE_INIT };
		RangingSessionState mRangingSessionState{ RangingSessionState::Uninitialized };
	};

	static constexpr size_t kCurrentDistanceBufferSize{ sizeof(
		cherry_ccc_session_controlee_measurements::distance_cm) };

	UltraWideBandImpl() = default;
	~UltraWideBandImpl() = default;

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
	 * @return ALIRO_NO_ERROR on success, error code otherwise.
	 */
	AliroError HandleDeviceCapsEvent(CoreEvent *event);

	/**
	 * @brief Event handler for device info event.
	 *
	 * @param event Pointer to the core event containing device info data.
	 * @return ALIRO_NO_ERROR on success, error code otherwise.
	 */
	AliroError HandleDeviceInfoEvent(CoreEvent *event);

	/**
	 * @brief Common helper function to wait for and handle UWB events.
	 *
	 * This function encapsulates the common pattern of waiting for a UWB event,
	 * checking for timeout/error conditions, and processing the event via a callback.
	 *
	 * @param expectedEvent The event type to wait for.
	 * @param eventHandler Function pointer to process the event data when received.
	 *
	 * @return ALIRO_NO_ERROR on success, error code otherwise.
	 */
	AliroError HandleUwbEvent(UwbEvents expectedEvent, EventHandler eventHandler);

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
	 * @param sessionCtx The session context to add.
	 *
	 * @return ALIRO_NO_ERROR on success, error code otherwise.
	 */
	AliroError AddSession(const SessionContext &sessionCtx);

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

	CoreEvent *mCoreEvent{};
	Callbacks mCallbacks{};
	cherry *mCtx{};
	aliro_uwb_adapter *mAliroCtx{};
	std::unique_ptr<char[]> mQm35FirmwareVersion{ nullptr };
	ActiveSessionsList mActiveSessionsList{};
	k_mutex mMutex{};
	std::array<uint8_t, kCurrentDistanceBufferSize> mCurrentDistanceCm{};

	aliro_uwb_adapter_reader_config mReaderConfig = {
		.min_ran_multiplier = CONFIG_DOOR_LOCK_UWB_MIN_RAN_MULTIPLIER,
		.preferred_hopping_configs = { .configs = { ALIRO_HOPPING_CONFIG_DISABLED,
							    ALIRO_HOPPING_CONFIG_CONTINUOUS_DEFAULT },
					       .count = 2 },
		.mac_mode = static_cast<uint8_t>(CONFIG_DOOR_LOCK_UWB_MAC_MODE_OFFSET |
						 (CONFIG_DOOR_LOCK_UWB_MAC_MODE_RANGING_ROUNDS << 6)),
	};
};

} // namespace Aliro::Uwb
