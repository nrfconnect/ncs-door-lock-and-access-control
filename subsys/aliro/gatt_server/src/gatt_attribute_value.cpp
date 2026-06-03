/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "gatt_attribute_value.h"

#include <doorlock/utils/memory.h>
#include <doorlock/utils/utils.h>

#include <zephyr/logging/log.h>
#include <zephyr/sys/byteorder.h>

#include <cstring>

LOG_MODULE_DECLARE(GattServer, CONFIG_DOOR_LOCK_ALIRO_GATT_SERVER_LOG_LEVEL);

namespace DoorLock::GattServer {

int GattAttributeValue::Generate(uint16_t spsm, const Aliro::ProtocolVersion *protocolVersions, size_t versionCount,
				 const SupportedFeatures &supportedFeatures)
{
	VerifyOrReturnValue(protocolVersions != nullptr && versionCount > 0, -EINVAL, LOG_ERR("Invalid arguments"));

	const size_t protocolVersionDataSize = versionCount * sizeof(Aliro::ProtocolVersion);

	// [SPSM:2][ProtocolVersionLen:1][ProtocolVersions:N*2][FeaturesLen:1][Features:1]
	const size_t totalSize = kSpsmSize + sizeof(uint8_t) + protocolVersionDataSize + kSupportedFeaturesLength +
				 kSupportedFeaturesSize;

	auto buffer = GattAttributePtr(DoorLock::Utils::new_array_nothrow<uint8_t>(totalSize));
	VerifyOrReturnValue(buffer != nullptr, -ENOMEM, LOG_ERR("Failed to allocate memory"));

	size_t offset{ 0 };

	sys_put_be16(spsm, buffer.get() + offset);
	offset += kSpsmSize;

	buffer[offset] = static_cast<uint8_t>(protocolVersionDataSize);
	offset += sizeof(uint8_t);

	for (size_t i = 0; i < versionCount; i++) {
		sys_put_be16(protocolVersions[i], buffer.get() + offset + (i * sizeof(Aliro::ProtocolVersion)));
	}
	offset += protocolVersionDataSize;

	buffer[offset] = kSupportedFeaturesSize;
	offset += sizeof(uint8_t);

	memcpy(buffer.get() + offset, &supportedFeatures, kSupportedFeaturesSize);

	// Store the buffer and size
	mGattAttributeBuffer = std::move(buffer);
	mGattAttributeBufferSize = static_cast<uint16_t>(totalSize);

	return 0;
}

} // namespace DoorLock::GattServer
