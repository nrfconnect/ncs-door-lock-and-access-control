/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include "crypto/backend_crypto_psa/crypto_psa.h"
#include "crypto/crypto.h"

namespace Aliro {

class CryptoImpl final : public Crypto, public CryptoPsa {
	friend class Crypto;
};

/* Implementation of the generic getter. */
inline Crypto &CryptoInstance()
{
	static CryptoImpl sCrypto;
	return sCrypto;
}

} // namespace Aliro
