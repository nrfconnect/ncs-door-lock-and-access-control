/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

#pragma once

#include "isodep_config.h"

#include "transport/data.h"

#include "aliro/errors.h"

namespace Aliro {

class IsoDepImpl;

/**
 * @class IsoDep
 * @brief Interface class for handling NFC ISO-DEP layer.
 *
 * IsoDep provides an interface for NFC data exchange based on the ISO-DEP protocol.
 * It allows for initialization, data preparation, and handling of ISO-DEP communication events.
 */
class IsoDep {
public:
	/**
	 * @enum SelectStatus
	 * @brief Status codes for tag selection.
	 */
	enum SelectStatus : uint8_t {
		/** Selection successful. */
		StatusOk,
		/** Bitrate divisor unsupported. */
		UnsupportedBitrateDivisor
	};

	/**
	 * @struct Callbacks
	 * @brief Struct containing callback functions for NFC events.
	 *
	 * This struct holds pointers to functions that are called on specific NFC events.
	 */
	struct Callbacks {
		/** Called when TX data is ready to be sent over the transport medium. */
		void (*mOnTxDataReady)(Data, uint32_t maxFrameDelayTime){ nullptr };
		/** Called when new processed data (RX) is available for reading. */
		void (*mOnRxDataAvailable)(Data){ nullptr };
		/** Called when a tag is selected. */
		void (*mOnTagSelected)(SelectStatus){ nullptr };
		/** Called on each error occurrence. */
		void (*mOnError)(AliroError){ nullptr };
	};

	/**
	 * @brief Initializes the IsoDep component with specified callbacks.
	 * @param callbacks Struct containing callback functions.
	 * @return AliroError status of the initialization.
	 */
	AliroError Init(Callbacks callbacks);

	/**
	 * @brief Prepares data for transmission. Once data is ready to be sent, the mOnTxDataReady callback must be
	 * called, so that the underlying driver can send the data through the transport medium.
	 * @param data Data to be prepared.
	 * @return AliroError status of the operation.
	 */
	AliroError PrepareData(Data data);

	/**
	 * @brief Prepares the RATS (Request for Answer To Select) command payload. Once data is ready to be sent,
	 * the mOnTxDataReady callback must be called, so that the underlying driver can RATS command through the
	 * transport medium.
	 * @return AliroError status of the operation.
	 */
	AliroError PrepareRats();

	/**
	 * @brief Handles the incoming data in the ISO-DEP layer. This function is supposed to be called with RX data
	 * forwarded from the transport medium.
	 * @param data Received data.
	 * @param transferError Error status of the data transfer.
	 * @return AliroError status of the operation.
	 */
	AliroError HandleReceivedData(Data data, int transferError);

	/**
	 * @brief Reports a timeout event. This function is supposed to be called to notify the ISO-DEP layer about the
	 * timeout that happened in the transport medium layer.
	 * @return AliroError status of the operation.
	 */
	AliroError ReportTimeout();

protected:
	IsoDep() = default;
	IsoDep(const IsoDep &) = delete;
	IsoDep(IsoDep &&) = delete;
	~IsoDep() = default;
	IsoDep &operator=(const IsoDep &) = delete;
	IsoDep &operator=(IsoDep &&) = delete;

	Callbacks mCallbacks{};

private:
	IsoDepImpl *Impl();
};

/**
 * @brief Returns the unique IsoDep implementation object that can be used by the client code.
 * @return Unique IsoDep object reference.
 */
extern IsoDep &IsoDepInstance();

} // namespace Aliro
