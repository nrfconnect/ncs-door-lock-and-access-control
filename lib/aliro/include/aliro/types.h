/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

#pragma once

#include <array>
#include <cstdint>
#include <cstring>
#include <optional>

/**
 * @brief Aliro specific common types definitions.
 *
 * Contains type definitions, constants, and utility functions
 * used commonly across the Aliro stack.
 */
namespace Aliro {

/**
 * @brief Length of the Reader group identifier.
 */
constexpr size_t kReaderGroupIdentifierLength{ 16 };

/**
 * @brief Length of the Reader group sub-identifier.
 */
constexpr size_t kReaderGroupSubIdentifierLength{ 16 };

/**
 * @brief Length of the Reader identifier.
 */
constexpr size_t kReaderIdentifierLength{ kReaderGroupIdentifierLength + kReaderGroupSubIdentifierLength };

/**
 * @brief Type alias for Reader identifier.
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

/**
 * @enum RangingSessionState
 * @brief Enumerates the possible states of a ranging session.
 */
enum class RangingSessionState : uint8_t {
	Uninitialized = 0x00,
	Initialized,
	Idle,
	Ranging,
	RangingSuspended,
	RangingResumed,
	Destroyed,
};

/**
 * @enum ReaderStateByte
 * @brief Enumerates the possible states of a reader written as a single byte.
 */
enum class ReaderStateByte : uint8_t {
	Secured = 0x00,
	Unsecured = 0x01,
	Blocked = 0x02,
	EnteringSecured = 0x80,
	EnteringUnsecured = 0x81,
	Unknown = 0x82
};

/**
 * @enum OperationSource
 * @brief Defines the operation source information in State Attribute ID.
 */
enum class OperationSource : uint8_t {
	Unspecified = 0x00,
	Manual,
	Auto,
	Schedule,
	ThisUserDeviceInBluetoothLeUwbAliroFlow,
	ThisUserDeviceInNfc,
	ThisUserDeviceInBluetoothLeOnlyFlow,
	Matter,
	// RFU 8 - 255
};

/**
 * @brief Type alias for Validity Iteration.
 */
using ValidityIteration = uint64_t;

/**
 * @brief Length of the timestamp.
 */
constexpr size_t kTimestampLength{ 20 };

/**
 * @brief Type alias for timestamp.
 */
using Timestamp = std::array<uint8_t, kTimestampLength>;

} // namespace Aliro

namespace Aliro::CryptoTypes {
/**
 * @brief Type alias for key identifiers.
 *
 * Used to uniquely identify cryptographic keys.
 */
using KeyId = uint32_t;

/**
 * @brief Length of an ECC P-256 private key.
 *
 * The number of bytes used for the private key.
 */
constexpr size_t kEccP256KeyPrivateKeyLength{ 32 };

/**
 * @brief Length of the signing key.
 *
 * The number of bytes used for the signing key.
 */
constexpr size_t kReaderSigningKeyLength{ kEccP256KeyPrivateKeyLength };

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
 * @brief Length of the ECC P-256 signature.
 *
 * Generate transaction data signature procedure.
 */
constexpr size_t kEccP256SignatureLength{ 64 };

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
 * @brief Encryption/decryption counter length - for nonce generation.
 */
constexpr size_t kEncryptionDecryptionCounterLength{ 4 };

/**
 * @brief Length of the nonce.
 *
 * IV <= 0x0000000000000001 || device_counter (unsigned big endian, 4 bytes)
 */
constexpr size_t kNonceLength{ sizeof(uint64_t) + kEncryptionDecryptionCounterLength };

/**
 * @brief Length of the transaction identifier.
 */
constexpr size_t kTransactionIdentifierLength{ 16 };

/**
 * @brief Length of the authentication tag.
 */
constexpr size_t kAuthenticationTagLength{ 16 };

/**
 * @brief Length of the Group Resolving Key.
 *
 * The length of the (symmetric) Group Resolving Key.
 */
constexpr size_t kGroupResolvingKeyLength{ 16 };

/**
 * @brief Type alias for ECC P-256 public key x coordinate.
 */
using PublicKeyXcoordinate = std::array<uint8_t, kEccP256KeySingleCoordinateLength>;

/**
 * @brief Type alias for ECC P-256 public key storage.
 *
 * A fixed-size array that can hold a complete ECC P-256 public key
 * in uncompressed format (prefix + x coordinate + y coordinate).
 */
using PublicKey = std::array<uint8_t, kEccP256PublicKeyLength>;

/**
 * @brief Type alias for ECC P-256 private key storage.
 *
 * A fixed-size array that can hold a complete ECC P-256 private key.
 */
using PrivateKey = std::array<uint8_t, kEccP256KeyPrivateKeyLength>;

/**
 * @brief Type alias for Ultra-wideband Ranging Session Key (URSK).
 * @brief Type alias for ultra wideband Ranging Session Key (URSK).
 *
 * A fixed-size array that holds the URSK used for UWB ranging sessions.
 * URSK is stored in plaintext and has the standard symmetric key length.
 */
using Ursk = std::array<uint8_t, kSymmetricKeyLength>;

/**
 * @brief Type alias for ECC P-256 signature.
 */
using Signature = std::array<uint8_t, kEccP256SignatureLength>;

/**
 * @brief Type alias for nonce.
 */
using Nonce = std::array<uint8_t, kNonceLength>;

/**
 * @brief Type alias for transaction identifier.
 */
using TransactionIdentifier = std::array<uint8_t, kTransactionIdentifierLength>;

/**
 * @brief Type alias for authentication tag.
 */
using AuthenticationTag = std::array<uint8_t, kAuthenticationTagLength>;

/**
 * @brief Type alias for Group Resolving Key.
 */
using GroupResolvingKey = std::array<uint8_t, kGroupResolvingKeyLength>;

/**
 * @brief Length of the key identifier.
 */
constexpr size_t kKeyIdentifierLength{ 8 };

/**
 * @brief Type alias for key identifier ("kid").
 */
using KeyIdentifier = std::array<uint8_t, kKeyIdentifierLength>;

/**
 * @brief Length of the SHA-256 hash.
 */
constexpr size_t kSha256HashLength{ 32 };

/**
 * @brief Type alias for SHA-256 hash.
 */
using Sha256Hash = std::array<uint8_t, kSha256HashLength>;

/**
 * @brief Type alias for key exchange shared secret.
 */
using SharedSecret = std::array<uint8_t, kEccP256KeySingleCoordinateLength>;

} // namespace Aliro::CryptoTypes

namespace Aliro::AccessDocumentTypes {

/**
 * @brief Document type.
 */
enum class DocumentType {
	Access,
	Revocation,
};

/**
 * @brief Structure representing an access document data.
 * The mPublicKey is the public key retrieved from the IssuerAuth structure in the Access Document.
 * The mDataElement contains data elements (IssuerSignedItems) retrieved from the Access Document.
 */
struct AccessDocument {
	const CryptoTypes::PublicKey &mPublicKey;
	ConstData mDataElement;
	const CryptoTypes::PublicKey &mCredentialIssuerPublicKey;
	const Timestamp &mSignedTimestamp;
	std::optional<uint64_t> mValidityIteration;
};

} // namespace Aliro::AccessDocumentTypes
