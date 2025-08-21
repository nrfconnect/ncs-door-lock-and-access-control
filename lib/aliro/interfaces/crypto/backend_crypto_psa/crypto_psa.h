/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
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
	AliroError _GenerateRandom(uint8_t *rngBuf, size_t rngBufLength);
	AliroError _GenerateEphemeralKeyPair(psa_key_id_t &keyId);
	AliroError _ExportPublicKey(psa_key_id_t keyId, EccP256PublicKey &publicKey);
	AliroError _ImportPublicKey(const EccP256PublicKey &staticPubKey, psa_key_id_t &keyId);
	AliroError _ImportPrivateKey(const EccP256PrivateKey &privateKey, psa_key_id_t &privateKeyId);
	AliroError _DestroyKey(psa_key_id_t &keyId) const;
	AliroError _GenerateSignature(psa_key_id_t privateKeyId, const uint8_t *msg, const size_t msgLength,
				      TransactionSignature &signature);
	AliroError _VerifySignature(psa_key_id_t publicKeyId, const uint8_t *msg, const size_t msgLength,
				    const TransactionSignature &signature);
	AliroError _ComputeSharedKeyDH(psa_key_id_t privKeyId, const EccP256PublicKey &publicKey,
				       const TransactionIdentifier &transactionId, psa_key_id_t &keyDhId);
	AliroError _DeriveSessionKeys(psa_key_id_t kDh, const KdfInfo &info, const KdfSalt &salt,
				      SessionBoundKeys &sessionVolatileKeys);
	AliroError _DeriveBleSessionKey(psa_key_id_t inputKeyId, const SharedByteSpan &info, const SharedByteSpan &salt,
					psa_key_id_t &outputKeyId);
	AliroError _EncryptPayload(psa_key_id_t keyId, const uint8_t *plainTxt, size_t plainTxtLength,
				   const uint8_t *additionalData, size_t additionalDataLength, const Nonce &nonce,
				   uint8_t *cipherText, AuthenticationTag &authTag);
	AliroError _EncryptPayload(psa_key_id_t keyId, const uint8_t *plainTxt, size_t plainTxtLength,
				   uint8_t *cipherText);
	AliroError _DecryptPayload(psa_key_id_t keyId, const uint8_t *cipherText, size_t cipherTextLength,
				   const uint8_t *additionalData, size_t additionalDataLength, const Nonce &nonce,
				   uint8_t *plainText, size_t &plainTextLength);
	AliroError _ProvisionSymmetricKey(const uint8_t *key, size_t keyLength, psa_key_id_t &keyId,
					  bool isPersistent = false);
	AliroError _IsKeyValid(psa_key_id_t keyId) const;
};

} /* namespace Aliro */
