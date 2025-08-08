/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

#pragma once

#include "aliro/errors.h"
#include "aliro/types.h"

#include <cstddef>

namespace Aliro::Uwb {

/**
 * @class UltraWideBand
 * @brief Interface class for managing ultra wideband (UWB) module and ranging session.
 *
 * This class provides methods to initialize, deinitialize, and manage UWB ranging sessions,
 * as well as handling BLE messages and time synchronization.
 */
template <typename IfaceImpl> class UltraWideBand {
public:
	using SessionIdentifier = uint32_t;
	using SessionContextHandle = const void *;

	/**
	 * @struct Callbacks
	 * @brief Struct containing callback functions for UWB events.
	 *
	 * This struct holds pointers to functions that are called on specific UWB events,
	 * such as transmitting BLE messages and receiving ranging data.
	 */
	struct Callbacks {
		/**
		 * @brief Callback to transmit a BLE message.
		 *
		 * This callback is invoked when the UWB module needs to send a BLE message
		 * as part of the Aliro protocol flow. The message must conform to the format
		 * specified in Table 11-10 of the Aliro specification, and the payload must stay
		 * unencrypted.
		 *
		 * @param sessionContextData Pointer to current session context data used by the Aliro stack.
		 * @param data Pointer to the formatted BLE message data.
		 * @param length Size of the message data in bytes.
		 */
		void (*mTransmitBleMessage)(SessionContextHandle sessionContextData, const uint8_t *data,
					    size_t length){ nullptr };
		/**
		 * @brief Callback to receive ranging data.
		 *
		 * This callback is invoked when the UWB module receives ranging data.
		 *
		 * @param sessionContextData Pointer to current session context data used by the Aliro stack.
		 * @param uwbData Ultra Wide Band ranging data.
		 */
		void (*mRangingData)(SessionContextHandle sessionContextData, const UwbRangingData &uwbData){ nullptr };
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
	 * @return ALIRO_NO_ERROR on success, or an error code on failure.
	 */
	AliroError Init(const Callbacks &callbacks) { return Impl()->_Init(callbacks); }

	/**
	 * @brief Deinitializes the UltraWideBand module.
	 *
	 * This method deinitializes the UWB hardware and releases any resources it was using.
	 *
	 * @return ALIRO_NO_ERROR on success, or an error code on failure.
	 */
	AliroError Deinit() { return Impl()->_Deinit(); };

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
	void BleTimeSync() { Impl()->_BleTimeSync(); }

	/**
	 * @brief Transmits deserialized and decrypted Aliro BLE messages to the UWB module.
	 *
	 * This method sends the UWB Ranging Service and Notifications (with ID == Ranging) to the UWB module.
	 * The message conform to the format specified in Table 11-10 of the Aliro specification, and the payload is
	 * decrypted.
	 *
	 * @param data Pointer to data to be transmitted.
	 * @param length Length of the data in bytes.
	 * @param sessionContextData Pointer to current session context data used by the Aliro stack.
	 *
	 * @return ALIRO_NO_ERROR if the transmission was successful, an error code otherwise.
	 */
	AliroError HandleBleMessage(const uint8_t *data, size_t length, SessionContextHandle sessionContextData)
	{
		return Impl()->_HandleBleMessage(data, length, sessionContextData);
	}

	/**
	 * @brief Configures the ranging session with the provided URSK in plaintext.
	 *
	 * This method sets up the ranging session using the provided URSK (UWB Ranging Secret Key).
	 *
	 * @param sessionId The session identifier for the ranging session (Chapter 11.7.2.1.3 in the Aliro spec.).
	 * @param ursk Reference to the URSK.
	 * @param sessionContextData Pointer to current session context data used by the Aliro stack.
	 *
	 * @return ALIRO_NO_ERROR on success, or an error code on failure.
	 */
	AliroError ConfigureRangingSession(SessionIdentifier sessionId, const CryptoTypes::Ursk &ursk,
					   SessionContextHandle sessionContextData)
	{
		return Impl()->_ConfigureRangingSession(sessionId, ursk, sessionContextData);
	}

	/**
	 * @brief Initiates a UWB ranging session.
	 *
	 * This method starts a UWB ranging session by creating the M1 message and sending it
	 * over the BLE interface to the User Device.
	 *
	 * @param sessionContextData Pointer to current session context data used by the Aliro stack.
	 *
	 * @return ALIRO_NO_ERROR if the session was initiated successfully, an error code otherwise.
	 */
	AliroError InitiateRangingSession(SessionContextHandle sessionContextData)
	{
		return Impl()->_InitiateRangingSession(sessionContextData);
	}

	/**
	 * @brief Terminates the current UWB ranging session.
	 *
	 * This method terminates the current UWB ranging session and releases any resources it was using.
	 *
	 * @param sessionContextData Pointer to current session context data used by the Aliro stack.
	 *
	 * @return ALIRO_NO_ERROR if the session was terminated successfully, an error code otherwise.
	 */
	AliroError TerminateRangingSession(SessionContextHandle sessionContextData)
	{
		return Impl()->_TerminateRangingSession(sessionContextData);
	}

	/**
	 * @brief Suspends the current UWB ranging session.
	 *
	 * This method suspends the current UWB ranging session. It can be used to pause the session temporarily, to
	 * resume the session use `AliroUwbResumeRangingSession()`.
	 *
	 * @param sessionContextData Pointer to current session context data used by the Aliro stack.
	 * @param force If true, forces the suspension even if there are ongoing operations.
	 *
	 * @return ALIRO_NO_ERROR if the session was suspended successfully, an error code otherwise.
	 */
	AliroError SuspendRangingSession(SessionContextHandle sessionContextData, bool force = false)
	{
		return Impl()->_SuspendRangingSession(sessionContextData, force);
	}

	/**
	 * @brief Resumes the suspended UWB ranging session.
	 *
	 * This method resumes the UWB ranging session that was previously suspended by
	 * `AliroUwbSuspendRangingSession()`.
	 *
	 * @param sessionContextData Pointer to current session context data used by the Aliro stack.
	 *
	 * @return ALIRO_NO_ERROR if the session was resumed successfully, an error code otherwise.
	 */
	AliroError ResumeRangingSession(SessionContextHandle sessionContextData)
	{
		return Impl()->_ResumeRangingSession(sessionContextData);
	}

protected:
	IfaceImpl *Impl() { return static_cast<IfaceImpl *>(this); }
	const IfaceImpl *Impl() const { return static_cast<const IfaceImpl *>(this); }
};

} // namespace Aliro::Uwb
