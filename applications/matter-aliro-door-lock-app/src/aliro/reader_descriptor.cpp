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

/** Matter Vendor ID, big-endian and zero-padded to the three-byte Reader Vendor ID field. */
constexpr std::array<uint8_t, Aliro::kReaderDescriptorVendorOuCidLength> kVendorOuCid{
	0x00,
	static_cast<uint8_t>(CONFIG_CHIP_DEVICE_VENDOR_ID >> 8),
	static_cast<uint8_t>(CONFIG_CHIP_DEVICE_VENDOR_ID),
};

/** Matter Product ID, big-endian. */
constexpr std::array<uint8_t, 2> kProductId{
	static_cast<uint8_t>(CONFIG_CHIP_DEVICE_PRODUCT_ID >> 8),
	static_cast<uint8_t>(CONFIG_CHIP_DEVICE_PRODUCT_ID),
};

} // namespace

Aliro::ReaderDescriptor GetReaderDescriptor()
{
	return {
		.mVendorOuCid = kVendorOuCid,
		.mProductId = { kProductId.data(), kProductId.size() },
		.mFirmwareVersion = { reinterpret_cast<const uint8_t *>(APP_VERSION_EXTENDED_STRING),
				      strlen(APP_VERSION_EXTENDED_STRING) },
	};
}

} // namespace DoorLock
