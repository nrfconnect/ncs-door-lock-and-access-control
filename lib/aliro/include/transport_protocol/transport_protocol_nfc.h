/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include "consts.h"
#include "crypto/crypto_access_protocol.h"
#include "errors.h"
#include "util/span.h"

namespace Aliro {

enum class AliroInstruction : uint8_t {
	kInsSelect = 0xA4,
	kInsEnvelope = 0xC3,
	kInsGetResponse = 0xC0,
	kInsAuth0 = 0x80,
	kInsLoadCer = 0xD1,
	kInsAuth1 = 0x81,
	kInsExchange = 0xC9,
	kInsControlFlow = 0x3C,
};

enum class AliroCfCmdS1Param : uint8_t {
	kTransFinishedWithFailure = 0x00,
};

enum class AliroCfCmdS2Param : uint8_t {
	kNoInfo = 0x00,
	kProtocolVersionNotSupported = 0x27,
};

enum class AliroNfcErrorStatusCode : uint32_t {
	kAliroNfcGenericError = 0x0000,
	// Status bytes returned as response from UD
	kAliroNfcConditionOfUseNotSatisfied = 0x6985,
};

struct NfcTpAliroCb {
	// Select phase: expedited or set-up.
	AliroError (*userDeviceDetected)(void);
	// Control Flow response received.
	AliroError (*communicationEnd)(void);
	// Communication error
	AliroError (*error)(AliroNfcErrorStatusCode statusCode);
	// Select Command response received.
	AliroError (*selectRecv)(uint8_t *rawData, size_t dataLen);
	// AUTH0 Command response received.
	AliroError (*auth0Recv)(uint8_t *rawData, size_t dataLen);
	// AUTH1 Command response received.
	AliroError (*auth1Recv)(uint8_t *rawData, size_t dataLen);
	// TODO: Additional callbacks
};

AliroError NfcTpAliroInit(NfcTpAliroCb *cb);
AliroError NfcTpAliroSelectExpPhase();
AliroError NfcTpAliroSelectStepUpPhase();
AliroError NfcTpAliroAuth0(uint8_t *auth0, uint16_t auth0Len);
AliroError NfcTpAliroAuth1(uint8_t *auth1, uint16_t auth1Len);
AliroError NfcTpAliroControlFlow(uint8_t *controlFlow, uint16_t controlFlowLen);

} /* namespace Aliro */
