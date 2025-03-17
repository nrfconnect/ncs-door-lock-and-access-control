/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include "crypto/crypto_common.h"
#include "crypto/crypto_key_cache.h"

#include <psa/crypto.h>

namespace Aliro {

/** Concrete implementation of Crypto component for specific backend. */
class CryptoPsa {
protected:
	AliroError _Init();
	AliroError _GenerateRandom(Byte *rngBuf, size_t rngBufLength);
	AliroError _GenerateEphemeralKeyPair(psa_key_id_t &keyId);
	AliroError _ExportPublicKey(psa_key_id_t keyId, EccP256PublicKey &publicKey);
	AliroError _ImportPublicKey(const EccP256PublicKey &staticPubKey, psa_key_id_t &keyId);
	AliroError _ImportPrivateKey(const EccP256PrivateKey &privateKey, psa_key_id_t &privateKeyId);
	AliroError _DestroyKey(psa_key_id_t &keyId) const;
	AliroError _GenerateSignature(const uint8_t *msg, const size_t msgLength, TransactionSignature &signature,
				      psa_key_id_t privateKeyId);
	AliroError _VerifySignature(const uint8_t *msg, const size_t msgLength, const TransactionSignature &signature,
				    psa_key_id_t spublicKeyId);
	AliroError _ComputeSharedKeyDH(psa_key_id_t privKeyId, const EccP256PublicKey &publicKey,
				       const TransactionIdentifier &transactionId, psa_key_id_t &keyDhId);
	AliroError _DeriveSessionKeys(psa_key_id_t kDh, const KdfInfo &info, const KdfSalt &salt,
				      SessionBoundKeys &sessionVolatileKeys);
	AliroError _EncryptPayload(const Byte *plainTxt, size_t plainTxtLength, const Byte *aad, size_t aadLength,
				   psa_key_id_t keyId, const Nonce &nonce, Byte *cipherText,
				   AuthenticationTag &authTag);
	AliroError _EncryptPayload(const Byte *plainTxt, size_t plainTxtLength, psa_key_id_t keyId, Byte *cipherText);
	AliroError _DecryptPayload(psa_key_id_t keyId, const Byte *cipherText, size_t cipherTextLength,
				   const Byte *additionalData, size_t additionalDataLength, const Nonce &nonce,
				   Byte *plainText, size_t &plainTextLength);
	AliroError _ProvisionSymmetricKey(const uint8_t *key, size_t keyLength, psa_key_id_t keyId,
					  bool isPersistent = false);
};

} /* namespace Aliro */
