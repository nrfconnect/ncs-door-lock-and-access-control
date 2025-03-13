/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include "nfc_driver_config.h"

#include "aliro/errors.h"
#include "transport_nfc/data.h"

namespace Aliro {

class NfcDriverImpl;

/**
 * @class NfcDriver
 * @brief Interface class for handling NFC reader driver.
 *
 * NfcDriver provides an interface for NFC data exchange and control of the NFC field and detection mechanisms for a
 * reader device.
 */
class NfcDriver {
public:
	/**
	 * @struct Callbacks
	 * @brief Struct containing callback functions for NFC  reader driver events.
	 *
	 * This struct holds pointers to functions that are called on specific NFC events.
	 */
	struct Callbacks {
		/** Called when data is received (RX). */
		void (*mOnDataReceived)(NfcTransport::Data, int transferError){ nullptr };
		/** Called when Tag4 is detected, anticollision is completed and ISO-DEP protocol can be activated. */
		void (*mOnTagDetected)(){ nullptr };
		/** Called when data is successfully sent. */
		void (*mOnSendSuccess)(){ nullptr };
		/** Called on each error occurrence. */
		void (*mOnError)(AliroError){ nullptr };
		/** Called on timeout, indicating if the system is in sleep. */
		void (*mOnTimeout)(bool inSleep){ nullptr };
	};

	/**
	 * @brief Initializes the NFC driver with specified callbacks.
	 * @param callbacks Struct containing callback functions.
	 * @return AliroError status of the initialization.
	 */
	AliroError Init(Callbacks callbacks);

	/**
	 * @brief Sends data via NFC to the detected tag.
	 * @param data Data to be sent.
	 * @param maximumFrameDelayTime Maximum frame delay time allowed for the transmission.
	 * @return AliroError status of the operation.
	 */
	AliroError Send(NfcTransport::Data data, uint32_t maximumFrameDelayTime);

	/**
	 * @brief Enables the NFC.
	 * @return AliroError status of the operation.
	 */
	AliroError NfcOn();

	/**
	 * @brief Disables the NFC.
	 * @return AliroError status of the operation.
	 */
	AliroError NfcOff();

protected:
	NfcDriver() = default;
	NfcDriver(const NfcDriver &) = delete;
	NfcDriver(NfcDriver &&) = delete;
	~NfcDriver() = default;
	NfcDriver &operator=(const NfcDriver &) = delete;
	NfcDriver &operator=(NfcDriver &&) = delete;

	Callbacks mCallbacks{};

private:
	NfcDriverImpl *Impl();
};

/**
 * @brief Returns the unique NfcDriver implementation object that can be used by the client code.
 * @return Unique NfcDriver object reference.
 */
extern NfcDriver &NfcDriverInstance();

} // namespace Aliro
