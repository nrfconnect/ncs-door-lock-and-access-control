/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

/**
 * @file nonce.h
 * @brief Nonce generation for External NVS AEAD operations.
 *
 * Provides a 96-bit (12-byte) nonce suitable for use with ChaCha20-Poly1305.
 * The nonce is seeded from a random value at initialization and then
 * monotonically incremented on each call to @ref Generate, guaranteeing
 * uniqueness within a single boot cycle.
 */

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace DoorLock::ExternalNvs::Nonce {

/** @brief Size of the nonce in bytes (96 bits). */
constexpr size_t kNonceSize{ 12 };

/** @brief Type representing a 96-bit nonce. */
using Nonce = std::array<uint8_t, kNonceSize>;

/**
 * @brief Initialize the nonce generator.
 *
 * Initializes PSA Crypto and seeds the internal nonce state with a
 * cryptographically secure random value.
 *
 * @retval 0 on success.
 * @retval -ENODEV if PSA Crypto initialization fails.
 * @retval -EIO if random number generation fails.
 */
int Init();

/**
 * @brief Generate the next unique nonce.
 *
 * Increments the internal counter and copies the resulting 96-bit value
 * into @p nonce. Must be called after a successful @ref Init.
 *
 * @param[out] nonce Buffer that receives the generated nonce.
 *
 * @retval 0 on success.
 * @retval -ENODEV if the nonce generator has not been initialized.
 */
int Generate(Nonce &nonce);

} // namespace DoorLock::ExternalNvs::Nonce
