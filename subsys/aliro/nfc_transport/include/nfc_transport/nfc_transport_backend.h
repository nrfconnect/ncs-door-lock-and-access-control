/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include "aliro/errors.h"
#include "aliro/types.h"

/**
 * @brief NFC transport backend contract.
 *
 * A backend drives the NFC reader and reports reader events through the callbacks
 * registered with Init(). It does not interact with the Aliro stack directly.
 *
 * The backend is selected with the DOOR_LOCK_ALIRO_NFC_TRANSPORT_BACKEND Kconfig choice.
 * An out-of-tree backend defines all functions declared in this header.
 *
 * Threading: the backend invokes all callbacks from the Aliro workqueue context.
 * Send() and Terminate() are called from the same context.
 */
namespace Aliro::NfcTransport::Backend {

/**
 * @struct Callbacks
 * @brief Callbacks used by the backend to report NFC reader events.
 */
struct Callbacks {
	/**
	 * @brief Called when an ISO-DEP (ISO14443-4) device has been activated.
	 *
	 * @return ALIRO_NO_ERROR if the device is accepted. On error, the backend deactivates
	 *         the device and resumes polling.
	 */
	AliroError (*mOnDeviceDetected)(){ nullptr };

	/**
	 * @brief Called when a response has been received from the active device.
	 *
	 * @param data Received data. The buffer is only valid for the duration of the call.
	 */
	void (*mOnDataReceived)(Data data){ nullptr };

	/**
	 * @brief Called when communication with the active device has been lost.
	 *
	 * This includes a failed data exchange and an empty response.
	 */
	void (*mOnDeviceLost)(){ nullptr };
};

/**
 * @brief Initializes the NFC reader.
 *
 * @param callbacks Callbacks used to report NFC reader events.
 * @return ALIRO_NO_ERROR on success, error code otherwise.
 */
AliroError Init(const Callbacks &callbacks);

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
 * @brief Sends data to the active device.
 *
 * Only valid while a device is active. The response is reported through
 * Callbacks::mOnDataReceived.
 *
 * @param data The data to send.
 * @return ALIRO_NO_ERROR on success, error code otherwise.
 */
AliroError Send(Data data);

/**
 * @brief Deactivates the active device and resumes polling.
 *
 * The operation is asynchronous.
 *
 * @return ALIRO_NO_ERROR on success, error code otherwise.
 */
AliroError Terminate();

} // namespace Aliro::NfcTransport::Backend
