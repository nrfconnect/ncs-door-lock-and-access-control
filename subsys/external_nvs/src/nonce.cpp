/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "nonce.h"

#include <algorithm>

#include <psa/crypto.h>
#include <zephyr/logging/log.h>

LOG_MODULE_DECLARE(external_nvs, CONFIG_DOOR_LOCK_EXTERNAL_NVS_LOG_LEVEL);

namespace DoorLock::ExternalNvs::Nonce {

namespace {

struct NonceCtx {
	uint64_t mLow;
	uint32_t mHigh;
} __packed;

static_assert(sizeof(NonceCtx) == kNonceSize, "NonceCtx must be 12 bytes");

NonceCtx sNonceCtx;
bool sInitialized{ false };

} // namespace

int Init()
{
	auto status = psa_crypto_init();
	if (status != PSA_SUCCESS) {
		LOG_ERR("Failed to initialize PSA Crypto: %d", status);
		return -ENODEV;
	}

	/* Initialize nonce to a random value. */
	status = psa_generate_random(reinterpret_cast<uint8_t *>(&sNonceCtx), sizeof(sNonceCtx));
	if (status != PSA_SUCCESS) {
		LOG_ERR("Failed to generate random nonce: %d", status);
		return -EIO;
	}

	sInitialized = true;

	return 0;
}

int Generate(Nonce &nonce)
{
	if (!sInitialized) {
		LOG_ERR("Nonce is not initialized");
		return -ENODEV;
	}

	++sNonceCtx.mLow;

	if (sNonceCtx.mLow == 0) {
		++sNonceCtx.mHigh;
	}

	std::copy_n(reinterpret_cast<uint8_t *>(&sNonceCtx), kNonceSize, nonce.data());

	return 0;
}

} // namespace DoorLock::ExternalNvs::Nonce
