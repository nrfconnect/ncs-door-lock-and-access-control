/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include <cstddef>
#include <cstdint>

#include "errors.h"
#include "util/span.h"
#include "util/static_byte_span.h"

namespace Aliro {

/**
 * @class ProtocolVersionArbiter
 * @brief Manages access protocol version negotiation between a user device and a reader.
 *
 * This class is responsible for determining the highest common access protocol version
 * based on the versions supported by both the user device and the reader.
 */
class ProtocolVersionArbiter {
public:
	using ProtocolVersionType = uint16_t;
	using ProtocolVersionList = const ProtocolVersionType *;
	using ProtocolVersionSpan = StaticByteSpan<sizeof(ProtocolVersionType)>;

	/**
	 * @brief Default constructor for ProtocolVersionArbiter.
	 *
	 * Initializes a ProtocolVersionArbiter instance with empty version lists for both the user device and the
	 * reader. This constructor is typically used when the version lists must be set later.
	 */
	ProtocolVersionArbiter() = default;

	/**
	 * @brief Initializes the protocol version arbiter with supported protocol versions from both the user device
	 * and the reader.
	 *
	 * This function sets up the arbiter with arrays of protocol versions that are supported by the user device and
	 * the reader. Aliro spec. (0.9.0) 10.2.1.2: The supported protocol versions MUST be provided in arrays ordered
	 * from the highest to the lowest version number.
	 *
	 * @param userDeviceVersions Pointer to an array of protocol versions supported by the user device.
	 * @param userDeviceVersionsNum The number of entries in the userDeviceVersions array.
	 * @param readerVersions Pointer to an array of protocol versions supported by the reader.
	 * @param readerVersionsNum The number of entries in the readerVersions array.
	 * @return ALIRO_NO_ERROR on success, ALIRO_INVALID_ARGUMENT on failure.
	 */
	AliroError Init(const ProtocolVersionType *userDeviceVersions, size_t userDeviceVersionsNum,
			const ProtocolVersionType *readerVersions, size_t readerVersionsNum);

	/**
	 * @brief Retrieves the current protocol version.
	 *
	 * This method returns the current protocol version stored in the arbiter.
	 * The version is returned as a ProtocolVersionType, which is a numeric representation.
	 *
	 * @return ProtocolVersionType The current protocol version.
	 */
	ProtocolVersionType GetCurrentVersion() const;

	/**
	 * @brief Retrieves the current protocol version as a span of bytes.
	 *
	 * This method constructs and returns a ProtocolVersionSpan, which encapsulates
	 * the current protocol version in a byte array format. This is useful for operations
	 * that require the protocol version to be in a byte-wise format.
	 *
	 * @return ProtocolVersionSpan A span object containing the byte array of the current protocol version.
	 */
	ProtocolVersionSpan GetCurrentVersionSpan() const;

	/**
	 * @brief Checks if the reader and the user device have common protocol version supported.
	 *
	 * This method determines if valid supported version is currently set.
	 *
	 * @return bool True if valid supported version is currently set, false otherwise.
	 */
	bool HasSupportedVersion() const { return mCurrentVersion != kInvalidVersion; }

private:
	ProtocolVersionType mCurrentVersion{ kInvalidVersion };

	/* spec 8.3.3.2.5: "If expedited_phase_supported_protocol_versions includes 0x0100, the Reader SHALL select
	   expedited protocol version 0x0100." */
	static constexpr ProtocolVersionType kPreferredVersion{ 0x0100 };

	static constexpr ProtocolVersionType kInvalidVersion{ 0x0000 };
};

} // namespace Aliro
