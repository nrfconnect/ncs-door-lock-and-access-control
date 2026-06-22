/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include <gatt_server/gatt_server.h>

#include <aliro/protocol_version.h>

#include <cstdint>
#include <memory>

namespace DoorLock::GattServer {

/**
 * @brief GATT attributes value for the Aliro service.
 *
 * This structure defines the GATT attributes value for the Aliro service,
 * including the SPSM value, protocol version, and supported features.
 */
struct GattAttributeValue {
	// Features supported length field size.
	static constexpr size_t kSupportedFeaturesLength{ 1 };

	// Type alias for the unique_ptr
	using GattAttributePtr = std::unique_ptr<uint8_t[]>;

	/**
	 * @brief Generates and stores the GATT attribute buffer internally.
	 *
	 * @param spsm The SPSM value
	 * @param protocolVersions Array of protocol versions
	 * @param versionCount Number of protocol versions
	 * @param supportedFeatures Supported features bitmap
	 *
	 * @return 0 on success, a negative errno value otherwise
	 */
	int Generate(uint16_t spsm, const Aliro::ProtocolVersion *protocolVersions, size_t versionCount,
		     const SupportedFeatures &supportedFeatures);

	/**
	 * @brief Gets the generated buffer data.
	 *
	 * @return Pointer to the buffer data, or nullptr if not generated
	 */
	const uint8_t *GetBufferData() const { return mGattAttributeBuffer.get(); }

	/**
	 * @brief Gets the buffer size.
	 *
	 * @return Size of the buffer in bytes
	 */
	uint16_t GetBufferSize() const { return mGattAttributeBufferSize; }

	/**
	 * @brief Checks if the buffer is not null.
	 *
	 * @return true if buffer exists, false otherwise
	 */
	bool NotNull() const { return mGattAttributeBuffer != nullptr; }

private:
	static constexpr uint8_t kSpsmSize{ sizeof(uint16_t) };
	static constexpr uint8_t kSupportedFeaturesSize{ sizeof(SupportedFeatures) };

	GattAttributePtr mGattAttributeBuffer{};
	uint16_t mGattAttributeBufferSize{ 0 };
};

} // namespace DoorLock::GattServer
