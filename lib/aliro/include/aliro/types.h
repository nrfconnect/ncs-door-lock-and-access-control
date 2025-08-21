/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

#pragma once

#include <array>
#include <cstdint>
#include <cstring>

/**
 * @brief Aliro specific common types definitions.
 *
 * Contains type definitions, constants, and utility functions
 * used commonly across the Aliro stack.
 */
namespace Aliro {

/**
 * @brief Length of the reader group identifier.
 */
constexpr size_t kReaderGroupIdentifierLength{ 16 };

/**
 * @brief Length of the reader group sub-identifier.
 */
constexpr size_t kReaderGroupSubIdentifierLength{ 16 };

/**
 * @brief Length of the reader identifier.
 */
constexpr size_t kReaderIdentifierLength{ kReaderGroupIdentifierLength + kReaderGroupSubIdentifierLength };

/**
 * @brief Type alias for reader identifier.
 */
using Identifier = std::array<uint8_t, kReaderIdentifierLength>;

/**
 * @brief Type alias for data storage.
 *
 * A simple struct that holds a pointer to a byte array and its length.
 */
struct Data {
	uint8_t *mData{ nullptr };
	size_t mLength{ 0 };
};

/**
 * @brief Type alias for const data storage.
 *
 * A simple struct that holds a pointer to a const byte array and its length.
 */
struct ConstData {
	const uint8_t *mData{ nullptr };
	size_t mLength{ 0 };
};

/**
 * @brief Structure representing UWB ranging data.
 */
using UwbRangingData = ConstData;

} // namespace Aliro

namespace Aliro::CryptoTypes {
/**
 * @brief Type alias for key identifiers.
 *
 * Used to uniquely identify cryptographic keys.
 */
using KeyId = uint32_t;

/**
 * @brief ECC P-256 public key prefix byte.
 *
 * The standard prefix byte (0x04) used to indicate an uncompressed
 * ECC P-256 public key format.
 */
constexpr uint8_t kEccP256PublicKeyPrefix{ 0x04 };

/**
 * @brief Length of the ECC P-256 public key prefix.
 *
 * The number of bytes used for the public key prefix.
 */
constexpr size_t kEccP256PublicKeyPrefixLength{ 1 };

/**
 * @brief Length of a single ECC P-256 coordinate.
 *
 * Both x and y coordinates of an ECC P-256 public key have this length.
 * This corresponds to 32 bytes (256 bits) per coordinate.
 */
constexpr size_t kEccP256KeySingleCoordinateLength{ 32 };

/**
 * @brief Total length of an ECC P-256 public key.
 *
 * Calculated as: prefix (1 byte) + x coordinate (32 bytes) + y coordinate (32 bytes)
 * Total: 65 bytes for an uncompressed ECC P-256 public key.
 */
constexpr size_t kEccP256PublicKeyLength{ kEccP256PublicKeyPrefixLength + (2 * kEccP256KeySingleCoordinateLength) };

/**
 * @brief Symmetric key length used for channel encryption.
 *
 * Standard length for symmetric keys used in Aliro cryptographic operations.
 * This corresponds to 32 bytes (256 bits).
 */
constexpr size_t kSymmetricKeyLength{ 32 };

/**
 * @brief Type alias for ECC P-256 public key storage.
 *
 * A fixed-size array that can hold a complete ECC P-256 public key
 * in uncompressed format (prefix + x coordinate + y coordinate).
 */
using PublicKey = std::array<uint8_t, kEccP256PublicKeyLength>;

/**
 * @brief Type alias for Ultra-wideband Ranging Session Key (URSK).
 *
 * A fixed-size array that holds the URSK used for UWB ranging sessions.
 * URSK is stored in plaintext and has the standard symmetric key length.
 */
using Ursk = std::array<uint8_t, kSymmetricKeyLength>;

} // namespace Aliro::CryptoTypes
