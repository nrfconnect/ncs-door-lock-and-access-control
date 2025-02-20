/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include "consts.h"
#include "errors.h"
#include "util/span.h"
#include "util/static_byte_span.h"
#include "util/utils.h"
#include <array>

using KeyId = uint32_t;
using Byte = uint8_t;
using Size = uint8_t;

namespace Aliro {

// Private Key length
static constexpr Size kEccP256PrivateKeyLength = 32;

// Public key prefix
static constexpr Size kEccP256PublicKeyPrefixLength = 1;
// x and y coordinates both have the same length
static constexpr Size kEccP256PublicKeyCoordinateLength = 32;
static constexpr Size kEccP256PublicKeyCoordinateXLength = kEccP256PublicKeyCoordinateLength;
// Public key length = prefix || coordinate_x || coordinate_y.
static constexpr Size kEccP256PublicKeyLength = kEccP256PublicKeyPrefixLength + (2 * kEccP256PublicKeyCoordinateLength);
// The coordinate_x is placed in public key just after the prefix.
static constexpr Size kEccP256PublicKeyCoordinateXPosition = 1;
// Signature is two times longer than the private key
static constexpr Size kEccP256SignatureLength = 2 * kEccP256PrivateKeyLength;

static constexpr Size kAuthenticationTagLength = 16;
static constexpr Size kDeviceCounterLength = 4;
// IV <= 0x0000000000000001 || device_counter (unsigned big endian, 4 bytes) - spec 8.3.1.6
static constexpr Size kNonceLength = (sizeof(uint64_t) + (kDeviceCounterLength));

using PublicKey = StaticByteSpan<kEccP256PublicKeyLength>;
using PublicKeyXcoordinate = StaticByteSpan<kEccP256PublicKeyCoordinateXLength>;
using EccP256PublicKey = std::array<Byte, kEccP256PublicKeyLength>;
using EccP256PrivateKey = std::array<Byte, kEccP256PrivateKeyLength>;
using TransactionSignature = std::array<Byte, kEccP256SignatureLength>;
using KdfSalt = SharedByteSpan;
using KdfInfo = SharedByteSpan;
using TransactionIdentifier = StaticByteSpan<kTransactionIdentifierLength>;
using AuthenticationTag = std::array<Byte, kAuthenticationTagLength>;
using Nonce = std::array<Byte, kNonceLength>;

} // namespace Aliro
