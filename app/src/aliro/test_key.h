/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

#pragma once

#include "aliro/types.h"

/* ECDSA long term keys generated with openssl:
   openssl ecparam -name secp256k1 -genkey -noout -out ec-secp256k1-priv-key.pem
   openssl ec -in ec-secp256k1-priv-key.pem -pubout > ec-secp256k1-pub-key.pem
*/
constexpr Aliro::CryptoTypes::PrivateKey mPrivateKey =
	Aliro::CryptoTypes::PrivateKey{ 0xfd, 0xf7, 0x1a, 0x37, 0x14, 0xe0, 0x78, 0xc2, 0xc2, 0xfa, 0x90,
					0x7a, 0xe9, 0xac, 0xf6, 0x24, 0xaa, 0x98, 0xad, 0xd7, 0xed, 0xf7,
					0x50, 0x0e, 0x61, 0xcf, 0x8a, 0xf4, 0xcc, 0x5a, 0x70, 0xa9 };
