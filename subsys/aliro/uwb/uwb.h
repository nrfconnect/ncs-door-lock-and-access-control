/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

#pragma once

#include "aliro/connection_handle.h"
#include "aliro/protocol_version.h"
#include "aliro/types.h"

#include <cstddef>
#include <optional>

namespace Aliro::Uwb {

class UltraWideBandImpl;

/**
 * @class UltraWideBand
 * @brief Interface class for managing ultra wideband (UWB) module and ranging session.
 *
 * This class provides methods to initialize, deinitialize, and manage UWB ranging sessions,
 * as well as handling BLE messages and time synchronization.
 */
class UltraWideBand {
public:
	using SessionIdentifier = uint32_t;
	using SessionContextHandle = ConnectionHandle;

	/**
	 * @struct Callbacks
	 * @brief Struct containing callback functions for UWB events and BLE transport.
	 *
	 * This struct holds pointers to functions that are called on specific UWB events
	 * and to send UWB session traffic over BLE.
	 */
	struct Callbacks {
		/**
		 * @brief Callback to receive ranging data.
		 *
		 * This callback is invoked when the UWB module receives ranging data.
		 *
		 * @param sessionContextData Pointer to current session context data used by the Aliro stack.
		 * @param uwbData Ultra Wide Band ranging data.
		 */
		void (*mRangingData)(SessionContextHandle sessionContextData, const UwbRangingData &uwbData){ nullptr };

		/**
		 * @brief Callback to receive UWB ranging session state change.
		 *
		 * This callback is invoked when the UWB ranging session state changes.
		 *
		 * @param sessionContextData Pointer to current session context data used by the Aliro stack.
		 * @param state The new UWB ranging session state.
		 */
		void (*mRangingSessionStateChanged)(SessionContextHandle sessionContextData,
						    RangingSessionState state){ nullptr };

		/**
		 * @brief Callback to send UWB session payload over BLE.
		 *
		 * @param sessionContextData Session handle used by the Aliro stack.
		 * @param data Payload bytes.
		 * @param length Payload length.
		 */
		void (*mBleMessageTransmit)(SessionContextHandle sessionContextData, const uint8_t *data,
					    size_t length){ nullptr };
	};

	/**
	 * @brief Initializes the UltraWideBand module.
	 *
	 * This method can be used to initialize the UWB module, setting up necessary configurations
	 * and preparing it for use. It should be called before any other UWB operations
	 * are performed.
	 *
	 * @param callbacks Struct containing callback functions for UWB events.
	 *
	 * @return 0 on success, or a negative error code on failure.
	 */
	int Init(const Callbacks &callbacks);

	/**
	 * @brief Deinitializes the UltraWideBand module.
	 *
	 * This method deinitializes the UWB hardware and releases any resources it was using.
	 *
	 * @return 0 on success, or a negative error code on failure.
	 */
	int Deinit();

	/**
	 * @brief Triggers Bluetooth LE (BLE) and UWB time synchronization.
	 *
	 * This method is called just after BLE connection establishment (CONNECT_IND event). In the method
	 * implementaion the Procedure 0: Bluetooth LE (BLE) Timesync can be started according to the chapter 19.4.4.1
	 * in Digital Key Release 3.0 Technical Specification Version 1.1.0.
	 *
	 * @note In implementation of this method for example, the GPIO pin can be used to trigger time
	 * synchronnization between BLE and UWB modules.
	 */
	void BleTimeSync();

	/**
	 * @brief Transmits deserialized and decrypted Aliro BLE messages to the UWB module.
	 *
	 * This method sends the UWB Ranging Service and Notifications (with ID == Ranging) to the UWB module.
	 * The message conforms to the format specified in the Aliro spec, and the payload is
	 * decrypted.
	 *
	 * @param data Pointer to data to be transmitted.
	 * @param length Length of the data in bytes.
	 * @param sessionContextData Pointer to current session context data used by the Aliro stack.
	 *
	 * @return 0 if the transmission was successful, a negative error code otherwise.
	 */
	int HandleBleMessage(const uint8_t *data, size_t length, SessionContextHandle sessionContextData);

	/**
	 * @brief Configures the ranging session with the provided URSK in plaintext.
	 *
	 * This method sets up the ranging session using the provided URSK (UWB Ranging Secret Key).
	 *
	 * @param sessionId The session identifier for the ranging session.
	 * @param ursk Reference to the URSK.
	 * @param protocolVersion The protocol version to use for the ranging session.
	 * @param sessionContextHandle Pointer to current session context handle used by the Aliro stack.
	 *
	 * @return 0 on success, or a negative error code on failure.
	 */
	int ConfigureRangingSession(SessionIdentifier sessionId, const CryptoTypes::Ursk &ursk,
				    ProtocolVersion protocolVersion, SessionContextHandle sessionContextHandle);

	/**
	 * @brief Initiates a UWB ranging session.
	 *
	 * This method starts a UWB ranging session by creating the M1 message and sending it
	 * over the BLE interface to the User Device.
	 *
	 * @param sessionContextData Pointer to current session context data used by the Aliro stack.
	 *
	 * @return 0 if the session was initiated successfully, a negative error code otherwise.
	 */
	int InitiateRangingSession(SessionContextHandle sessionContextData);

	/**
	 * @brief Terminates the current UWB ranging session.
	 *
	 * This method terminates the current UWB ranging session and releases any resources it was using.
	 *
	 * @param sessionContextData Pointer to current session context data used by the Aliro stack.
	 *
	 * @return 0 if the session was terminated successfully, a negative error code otherwise.
	 */
	int TerminateRangingSession(SessionContextHandle sessionContextData);

	/**
	 * @brief Suspends the current UWB ranging session.
	 *
	 * This method suspends the current UWB ranging session. It can be used to pause the session temporarily, to
	 * resume the session use `AliroUwbResumeRangingSession()`.
	 *
	 * @param sessionContextData Pointer to current session context data used by the Aliro stack.
	 *
	 * @return 0 if the session was suspended successfully, a negative error code otherwise.
	 */
	int SuspendRangingSession(SessionContextHandle sessionContextData);

	/**
	 * @brief Resumes the suspended UWB ranging session.
	 *
	 * This method resumes the UWB ranging session that was previously suspended by
	 * `AliroUwbSuspendRangingSession()`.
	 *
	 * @param sessionContextData Pointer to current session context data used by the Aliro stack.
	 *
	 * @return 0 if the session was resumed successfully, a negative error code otherwise.
	 */
	int ResumeRangingSession(SessionContextHandle sessionContextData);

	/**
	 * @brief Gets the firmware version of the UWB module.
	 *
	 * @return Pointer to the firmware version string, or nullptr if not available.
	 */
	const char *GetFirmwareVersion();

	/**
	 * @brief Indicates whether the UWB module has been fully initialized.
	 *
	 * When true, device is ready to be used.
	 */
	bool IsInitialized();

	/**
	 * @brief Stops the UWB radar session.
	 */
	void StopRadarSession();

#ifdef CONFIG_DOOR_LOCK_ALIRO_UWB_QM35_DISAMBIGUATION
	/**
	 * @brief Returns the disambiguation session index associated with a session handle.
	 *
	 * @param sessionContextData Session context handle.
	 *
	 * @return Disambiguation session index if found, std::nullopt otherwise.
	 */
	std::optional<uint8_t> GetDisambiguationSessionIdx(SessionContextHandle sessionContextData);
#endif // CONFIG_DOOR_LOCK_ALIRO_UWB_QM35_DISAMBIGUATION

private:
	UltraWideBandImpl *Impl();
	const UltraWideBandImpl *Impl() const;
};

/**
 * @brief Get the singleton instance of the UltraWideBand interface.
 *
 * @return Reference to the UltraWideBand instance.
 */
UltraWideBand &UltraWideBandInstance();

} // namespace Aliro::Uwb
