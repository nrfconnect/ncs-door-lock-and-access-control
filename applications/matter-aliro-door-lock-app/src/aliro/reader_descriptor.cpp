/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "aliro/reader_descriptor.h"

#include <app_version.h>

#include <array>
#include <cstdint>
#include <cstring>

namespace DoorLock {

namespace {

/** Nordic Semiconductor IEEE OUI (first 3 bytes). */
constexpr std::array<uint8_t, Aliro::kReaderDescriptorVendorOuCidLength> kVendorOuCid{ 0xF4, 0xCE, 0x36 };

std::array<uint8_t, 2> MakeProductIdBytes()
{
	const uint16_t productId = CONFIG_CHIP_DEVICE_PRODUCT_ID;
	return { static_cast<uint8_t>(productId >> 8), static_cast<uint8_t>(productId) };
}

} // namespace

Aliro::ReaderDescriptor GetReaderDescriptor()
{
	static const std::array<uint8_t, 2> productId = MakeProductIdBytes();

	return {
		.mVendorOuCid = kVendorOuCid,
		.mProductId = { productId.data(), productId.size() },
		.mFirmwareVersion = { reinterpret_cast<const uint8_t *>(APP_VERSION_STRING),
				      strlen(APP_VERSION_STRING) },
	};
}

} // namespace DoorLock
