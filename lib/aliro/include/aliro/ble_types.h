/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

#pragma once

#include "aliro/protocol_version.h"

#include <zephyr/sys/byteorder.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>

/**
 * @brief Aliro BLE-specific type definitions.
 *
 * Contains BLE-specific type definitions, constants, and structures
 * used for BLE transport implementation in the Aliro stack.
 */
namespace Aliro::BleTypes {

/**
 * @brief Constant representing an invalid BLE UWB protocol version.
 */
constexpr ProtocolVersion kInvalidProtocolVersion{ 0x0000 };

/**
 * @brief Constant representing the size of the BLE address.
 */
constexpr size_t kBleAddressSize{ 6 };

/**
 * @typedef BleAddress
 * @brief Type alias for representing a BLE address.
 */
using BleAddress = std::array<uint8_t, kBleAddressSize>;

/**
 * @brief Aliro Service UUID assigned by Bluetooth SIG.
 */
constexpr uint16_t kAliroServiceUuid = 0xFFF2;

/**
 * @typedef BleExpiryTimestamp
 * @brief Type alias for representing an expiry timestamp.
 */
using BleExpiryTimestamp = uint32_t;

/**
 * @brief Constant representing an unavailable expiry timestamp.
 */
constexpr BleExpiryTimestamp kExpiryTimeUnavailable{ 0xFFFFFFFF };

/**
 * @typedef TxPowerLevel
 * @brief Type alias for representing the transmit (Tx) power level.
 */
using TxPowerLevel = int8_t;

/**
 * @brief BLE advertising service data structure.
 *
 * Contains all the data fields that are advertised in BLE advertising packets
 * according to the Aliro specification.
 */
struct AdvertisingServiceData {
	// Constants defining the sizes and masks for different fields according to the Aliro spec.
	constexpr static std::byte kAdvertisingVersionMask{ 0x07 };
	constexpr static std::byte kNotificationMask{ 0x03 };
	constexpr static uint8_t kMaxReaderGroupIdSize{ 8 };
	constexpr static uint8_t kMaxReaderGroupSubIdSize{ 2 };
	constexpr static uint8_t kMaxDynamicTagSize{ 7 };

	// Enumeration for notification types in advertising data.
	enum class Notification : uint8_t {
		NoError = 0,
		UnknownError,
		LowBattery,
		SensorTriggered,
	};

	// Flags bitfield definition
	struct ServiceFlags {
		uint8_t version : 3; // Bits [2:0]: Aliro BLE Advertisement Version
		uint8_t notification : 2; // Bits [4:3]: Notification
		uint8_t : 1; // Bit 5: Reserved for Future Use
		uint8_t : 1; // Bit 6: BLE-Only Aliro flow is not supported
		uint8_t bleUwb : 1; // Bit 7: BLE + UWB Aliro flow supported
	} __packed;

	// Aliro service payload of ADV_IND
	ServiceFlags mServiceFlags{ .bleUwb = 1 }; // Initialize with UWB support enabled by default
	TxPowerLevel mTxPowerLevelDbm{};
	uint8_t mTruncatedReaderGroupId[kMaxReaderGroupIdSize]{};
	uint8_t mTruncatedReaderGroupSubId[kMaxReaderGroupSubIdSize]{};
	uint8_t mDynamicTagExpiryTime[sizeof(BleExpiryTimestamp)]{};
	uint8_t mRfu{ 0 };
	uint8_t mDynamicTag[kMaxDynamicTagSize]{};

	/**
	 * @brief Sets the version field in the ServiceFlags bitfield.
	 *
	 * @param version the version value to set (3 bits, range: 0-7, where 1-7 are RFU).
	 */
	void SetVersion(uint8_t version);

	/**
	 * @brief Sets the notification field in the ServiceFlags bitfield.
	 *
	 * @param notification the notification type to set, based on the Notification enumeration.
	 */
	void SetNotification(Notification notification);

	/**
	 * @brief Sets the transmit power level in dBm.
	 *
	 * @param powerLevelDbm the transmit power level to set.
	 */
	void SetTxPowerLevel(TxPowerLevel powerLevelDbm);

	/**
	 * @brief Sets the truncated reader group ID.
	 *
	 * @param readerGroupId Pointer to the array containing the reader group ID.
	 *                      The size of the array should not exceed kMaxReaderGroupIdSize.
	 */
	void SetTruncatedReaderGroupId(const uint8_t *readerGroupId);

	/**
	 * @brief Sets the truncated reader group sub-ID.
	 *
	 * @param readerGroupSubId Pointer to the array containing the reader group sub-ID.
	 *                         The size of the array should not exceed kMaxReaderGroupSubIdSize.
	 */
	void SetTruncatedReaderGroupSubId(const uint8_t *readerGroupSubId);

	/**
	 * @brief Sets the dynamic tag expiry timestamp in Unix format (32-bit unsigned integer).
	 *
	 * @param expiryTimestampUnix the expiry timestamp in Unix format.
	 */
	void SetDynamicTagExpiryTimestamp(BleExpiryTimestamp expiryTimestampUnix);

	/**
	 * @brief Sets the dynamic tag.
	 *
	 * @param dynamicTag Pointer to the array containing the dynamic tag.
	 *                   The size of the array should not exceed kMaxDynamicTagSize.
	 */
	void SetDynamicTag(const uint8_t *dynamicTag);

} __attribute__((packed));

/**
 * @brief Aliro service structure for BLE advertising.
 *
 * This structure represents the complete Aliro service data that is
 * included in BLE advertising packets.
 */
struct AdvertisingService {
	// Aliro Service UUID assigned by Bluetooth SIG.
	uint8_t Uuid[2] = sys_uint16_to_array(kAliroServiceUuid);

	AdvertisingServiceData mAdvertisingServiceData{};
} __attribute__((packed));

} // namespace Aliro::BleTypes
