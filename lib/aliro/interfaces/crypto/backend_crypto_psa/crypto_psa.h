/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

#pragma once

#include "aliro/errors.h"
#include "aliro/types.h"

#include <cstddef>

namespace Aliro {

/** Concrete implementation of Crypto component for specific backend. */
class CryptoPsa {
protected:
	AliroError _Init();
	AliroError _GenerateRandom(uint8_t *rngBuf, size_t rngBufLength);
	AliroError _GenerateEphemeralKeyPair(CryptoTypes::KeyId &keyId);
	AliroError _ExportPublicKey(CryptoTypes::KeyId keyId, CryptoTypes::PublicKey &publicKey);
	AliroError _ExportKey(CryptoTypes::KeyId keyId, uint8_t *key, size_t keyLength);
	AliroError _ImportPublicKey(const CryptoTypes::PublicKey &key, CryptoTypes::KeyId &keyId);
	AliroError _ImportPrivateKey(const CryptoTypes::PrivateKey &key, CryptoTypes::KeyId &keyId, bool isPersistent);
	AliroError _PreserveKey(CryptoTypes::KeyId &volatileKeyId, CryptoTypes::KeyId persistentKeyId);
	AliroError _DestroyKey(CryptoTypes::KeyId &keyId) const;
	AliroError _GenerateSignature(CryptoTypes::KeyId privateKeyId, const uint8_t *msg, const size_t msgLength,
				      CryptoTypes::Signature &signature);
	AliroError _VerifySignature(CryptoTypes::KeyId publicKeyId, const uint8_t *msg, const size_t msgLength,
				    const CryptoTypes::Signature &signature);
	AliroError _ComputeSharedKeyDH(CryptoTypes::KeyId privKeyId, const CryptoTypes::PublicKey &publicKey,
				       const CryptoTypes::TransactionIdentifier &transactionId,
				       CryptoTypes::KeyId &keyDhId);
	AliroError _DeriveSessionKeys(CryptoTypes::KeyId kDh, const uint8_t *info, size_t infoLength,
				      const uint8_t *salt, size_t saltLength,
				      CryptoTypes::SessionBoundKeys &sessionVolatileKeys);
	AliroError _DeriveSessionKeysFromKpersistent(CryptoTypes::KeyId kpersistentKeyId, const uint8_t *info,
						     size_t infoLength, const uint8_t *salt, size_t saltLength,
						     CryptoTypes::SessionBoundKeys &sessionVolatileKeys);
	AliroError _DeriveBleSessionKey(CryptoTypes::KeyId inputKeyId, const uint8_t *info, size_t infoLength,
					const uint8_t *salt, size_t saltLength, CryptoTypes::KeyId &outputKeyId);
	AliroError _DeriveKpersistent(CryptoTypes::KeyId inputKeyId, const uint8_t *info, size_t infoLength,
				      const uint8_t *salt, size_t saltLength, CryptoTypes::KeyId &outputKeyId);
	AliroError _EncryptPayload(CryptoTypes::KeyId keyId, const uint8_t *plainTxt, size_t plainTxtLength,
				   const uint8_t *additionalData, size_t additionalDataLength,
				   const CryptoTypes::Nonce &nonce, uint8_t *cipherText,
				   CryptoTypes::AuthenticationTag &authTag);
	AliroError _EncryptPayload(CryptoTypes::KeyId keyId, const uint8_t *plainTxt, size_t plainTxtLength,
				   uint8_t *cipherText);
	AliroError _DecryptPayload(CryptoTypes::KeyId keyId, const uint8_t *cipherText, size_t cipherTextLength,
				   const uint8_t *additionalData, size_t additionalDataLength,
				   const CryptoTypes::Nonce &nonce, uint8_t *plainText, size_t &plainTextLength);
	AliroError _ProvisionSymmetricKey(const uint8_t *key, size_t keyLength, CryptoTypes::KeyId &keyId,
					  bool isPersistent = false);
	AliroError _IsKeyValid(CryptoTypes::KeyId keyId) const;
};

} /* namespace Aliro */
