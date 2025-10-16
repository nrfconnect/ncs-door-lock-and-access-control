/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "gatt_attribute_value.h"

#include "aliro/memory.h"
#include "aliro/utils.h"

#include <cstring>
#include <zephyr/logging/log.h>
#include <zephyr/sys/byteorder.h>

LOG_MODULE_DECLARE(GattServer, CONFIG_NCS_ALIRO_BLE_LOG_LEVEL);

namespace Aliro {

AliroError GattAttributeValue::Generate(L2capServer::Spsm spsm, const ProtocolVersion *protocolVersions,
					size_t versionCount, const SupportedFeatures &supportedFeatures)
{
	VerifyOrReturnStatus(protocolVersions != nullptr && versionCount > 0, ALIRO_INVALID_ARGUMENT,
			     LOG_ERR("Invalid arguments"));

	const size_t protocolVersionDataSize = versionCount * sizeof(ProtocolVersion);

	// [SPSM:2][ProtocolVersionLen:1][ProtocolVersions:N*2][FeaturesLen:1][Features:1]
	const size_t totalSize = kSpsmSize + sizeof(uint8_t) + protocolVersionDataSize + kSupportedFeaturesLength +
				 kSupportedFeaturesSize;

	auto buffer = GattAttributePtr(new_array_nothrow<uint8_t>(totalSize));
	VerifyOrReturnStatus(buffer != nullptr, ALIRO_NO_MEMORY, LOG_ERR("Failed to allocate memory"));

	size_t offset{ 0 };

	sys_put_be16(spsm, buffer.get() + offset);
	offset += kSpsmSize;

	buffer[offset] = static_cast<uint8_t>(protocolVersionDataSize);
	offset += sizeof(uint8_t);

	for (size_t i = 0; i < versionCount; i++) {
		sys_put_be16(protocolVersions[i], buffer.get() + offset + (i * sizeof(ProtocolVersion)));
	}
	offset += protocolVersionDataSize;

	buffer[offset] = kSupportedFeaturesSize;
	offset += sizeof(uint8_t);

	memcpy(buffer.get() + offset, &supportedFeatures, kSupportedFeaturesSize);

	// Store the buffer and size
	mGattAttributeBuffer = std::move(buffer);
	mGattAttributeBufferSize = static_cast<uint16_t>(totalSize);

	return ALIRO_NO_ERROR;
}

} // namespace Aliro
