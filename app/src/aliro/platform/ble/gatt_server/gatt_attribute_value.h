/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include "aliro/errors.h"
#include "aliro/protocol_version.h"
#include "l2cap_server/l2cap_server.h"

#include <memory>

namespace Aliro {

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

	// Supported features bitmap
	struct SupportedFeatures {
		uint8_t TimesyncProcedure0 : 1;
		uint8_t TimesyncProcedure1 : 1;
		uint8_t LeCodedPhy : 1;
		uint8_t : 5;
	} __packed;

	/**
	 * @brief Generates and stores the GATT attribute buffer internally.
	 *
	 * @param spsm The SPSM value
	 * @param protocolVersions Array of protocol versions
	 * @param versionCount Number of protocol versions
	 * @param supportedFeatures Supported features bitmap
	 *
	 * @return ALIRO_NO_ERROR on success, an error code otherwise
	 */
	AliroError Generate(L2capServer::Spsm spsm, const ProtocolVersion *protocolVersions, size_t versionCount,
			    const SupportedFeatures &supportedFeatures);

	/**
	 * @brief Generates and stores the GATT attribute buffer internally with default supported features.
	 *
	 * @param spsm The SPSM value
	 * @param protocolVersions Array of protocol versions
	 * @param versionCount Number of protocol versions
	 *
	 * @return ALIRO_NO_ERROR on success, an error code otherwise
	 */
	AliroError Generate(L2capServer::Spsm spsm, const ProtocolVersion *protocolVersions, size_t versionCount)
	{
		return Generate(spsm, protocolVersions, versionCount, kDefaultSupportedFeatures);
	}

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
	static constexpr uint8_t kSpsmSize{ sizeof(L2capServer::Spsm) };
	static constexpr uint8_t kSupportedFeaturesSize{ sizeof(SupportedFeatures) };
	static constexpr SupportedFeatures kDefaultSupportedFeatures{
		.TimesyncProcedure0 = IS_ENABLED(CONFIG_TIMESYNC_PROCEDURE_0) ? 1 : 0,
		.TimesyncProcedure1 = IS_ENABLED(CONFIG_TIMESYNC_PROCEDURE_1) ? 1 : 0,
		.LeCodedPhy = IS_ENABLED(CONFIG_BT_CTLR_PHY_CODED) ? 1 : 0,
	};

	GattAttributePtr mGattAttributeBuffer{};
	uint16_t mGattAttributeBufferSize{ 0 };
};

} // namespace Aliro
