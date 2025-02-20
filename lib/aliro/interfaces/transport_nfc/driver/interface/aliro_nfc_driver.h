/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include "nfc_driver_config.h"

#include "errors.h"
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
	 * @enum DetectType
	 * @brief Types of detection commands that can be sent to an NFC tag.
	 */
	enum DetectType : uint8_t {
		/** A command that requires the tag to be moved out of and back into the field to be re-detected. */
		REQA,
		/** A command that can wake the tag from a HALT state without a need to move it out of the field. */
		WUPA
	};

	/**
	 * @struct Callbacks
	 * @brief Struct containing callback functions for NFC  reader driver events.
	 *
	 * This struct holds pointers to functions that are called on specific NFC events.
	 */
	struct Callbacks {
		/** Called when data is received (RX). */
		void (*mOnDataReceived)(NfcTransport::Data, int transferError){ nullptr };
		/** Called when anticollision sequence is completed. */
		void (*mOnAnticollisionCompleted)(){ nullptr };
		/** Called when a tag is detected. */
		void (*mOnTagDetected)(){ nullptr };
		/** Called when the NFC field is turned on. */
		void (*mOnFieldOn)(){ nullptr };
		/** Called when data is successfully sent. */
		void (*mOnSendSuccess)(){ nullptr };
		/** Called on each error occurrence. */
		void (*mOnError)(AliroError){ nullptr };
		/** Called when the tag goes into sleep mode. */
		void (*mOnTagSleep)(){ nullptr };
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
	 * @brief Turns the reader's NFC field on.
	 * @return AliroError status of the operation.
	 */
	AliroError FieldOn();

	/**
	 * @brief Turns the the reader's NFC field off.
	 * @return AliroError status of the operation.
	 */
	AliroError FieldOff();

	/**
	 * @brief Starts the detection of the NFC tag using the specified detection type.
	 * @param type The type of detection command to use.
	 * @return AliroError status of the operation.
	 */
	AliroError TagDetect(DetectType type);

	/**
	 * @brief Starts the anticollision sequence.
	 * @return AliroError status of the operation.
	 */
	AliroError StartAnticollision();

	/**
	 * @brief Recovers the NFC driver polling after a specified delay.
	 * @param delayMs Delay in milliseconds before recovering.
	 * @param type Type of detection.
	 * @param tagSleep A flag indicating if the sleep command must be sent to the tag.
	 * @return AliroError Status of the restart.
	 */
	AliroError RecoverPolling(uint32_t delayMs, DetectType type, bool tagSleep);

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
