/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include <cstdint>

enum class Tag : uint8_t {
	/* SELECT response */
	FileControlInformation = 0x6F,
	ApplicationID = 0x84,
	ProprietaryInfo = 0xA5,
	Type = 0x80,
	/* 0x7F66 */
	ExtendedLengthInfo = 0x66,
	MaxCmdApdu = 0x02,
	MaxRespApdu = 0x02,
	VendorSpecificExtension = 0xB3,
	/* CONTROL FLOW command */
	S1Parameter = 0x41,
	S2Parameter = 0x42,
	/* AUTH0 command. */
	CommandParameters = 0x41,
	AuthenticationPolicy = 0x42,
	ExpeditedPhaseProtocolVersion = 0x5C,
	Reader_ePubKey = 0x87,
	TransactionIdentifier = 0x4C,
	ReaderIdentifier = 0x4D,
	CommandVendorExtension = 0xB1,
	UserDevice_ePubKey = 0x86,
	/* AUTH0 response. */
	Cryptogram = 0x9D,
	ResponseVendorExtension = 0xB2,
	/* AUTH1 command */
	ReaderSignature = 0x9E,
	/* Reader authentication. */
	Credential_ePubKx = 0x86,
	Reader_ePubKx = 0x87,
	Usage = 0x93,
	/* AUTH1 response */
	KeySlot = 0x4E,
	LongTermPublicKey = 0x5A,
	UserDeviceSignature = ReaderSignature,
	PrivateMailboxDataSubset = 0x4B,
	SignalingBitmap = 0x5E,
	CredentialSignedTimestamp = 0x91,
	RevocationSignedTimestamp = 0x92,
	/* Invalid */
	Invalid = 0xFF
};
