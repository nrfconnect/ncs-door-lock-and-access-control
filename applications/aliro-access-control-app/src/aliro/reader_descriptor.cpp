/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "aliro/reader_descriptor.h"

#include <app_version.h>

#include <array>
#include <cstring>

namespace DoorLock {

namespace {

/** Nordic Semiconductor IEEE OUI (first 3 bytes). */
constexpr std::array<uint8_t, Aliro::kReaderDescriptorVendorOuCidLength> kVendorOuCid{ 0xF4, 0xCE, 0x36 };

/** Sample product identifier for the Aliro-only door lock application. */
constexpr std::array<uint8_t, 2> kProductId{ 0x80, 0x07 };

} // namespace

Aliro::ReaderDescriptor GetReaderDescriptor()
{
	return {
		.mVendorOuCid = kVendorOuCid,
		.mProductId = { kProductId.data(), kProductId.size() },
		.mFirmwareVersion = { reinterpret_cast<const uint8_t *>(APP_VERSION_STRING),
				      strlen(APP_VERSION_STRING) },
	};
}

} // namespace DoorLock
