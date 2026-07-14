/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/ztest.h>

#include <aliro/interface.h>

#include <crypto_utils/crypto_utils.h>

#include <psa/crypto.h>
#include <zephyr/kvss/nvs.h>
#include <zephyr/kvss/zms.h>
#include <zephyr/settings/settings.h>

namespace {

constexpr size_t kSaltLength{ 141 };
constexpr size_t kInfoLength{ 32 };

std::array<uint8_t, kSaltLength> sTestSalt = {
	0x18, 0xA9, 0xF3, 0x12, 0x0A, 0xCC, 0xFB, 0xD9, 0xCC, 0x55, 0x31, 0x01, 0x88, 0x15, 0xEB, 0x78, 0xB9, 0x71,
	0x23, 0xF8, 0x76, 0x9C, 0x38, 0x9C, 0x1C, 0xF0, 0x11, 0xD7, 0x0A, 0x64, 0xF1, 0xF0, 0x56, 0x6F, 0x6C, 0x61,
	0x74, 0x69, 0x6C, 0x65, 0x2A, 0x2A, 0x2A, 0x2A, 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
	0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0xFF, 0xEE, 0xDD, 0xCC, 0xBB, 0xAA, 0x99, 0x88, 0x77, 0x66, 0x55, 0x44,
	0x33, 0x22, 0x11, 0x00, 0x5E, 0x5C, 0x02, 0x01, 0x00, 0x96, 0x96, 0xAF, 0xE3, 0x3D, 0xE5, 0x8B, 0x7D, 0x32,
	0x53, 0xD1, 0xCB, 0xA8, 0x6D, 0x14, 0x14, 0x7C, 0x16, 0xD4, 0x55, 0xE8, 0xA2, 0x73, 0x73, 0xB3, 0x8D, 0x45,
	0x4A, 0xF2, 0x1B, 0x70, 0xE7, 0x41, 0x65, 0xA8, 0x36, 0x67, 0xAD, 0x0A, 0xF5, 0xAB, 0x11, 0x52, 0x47, 0x42,
	0x48, 0x22, 0xE0, 0x00, 0x01, 0xA5, 0x08, 0x80, 0x02, 0x00, 0x00, 0x5C, 0x02, 0x01, 0x00
};

std::array<uint8_t, kInfoLength> sTestInfo = { 0xF8, 0x8D, 0xE8, 0x23, 0x25, 0xE1, 0xDE, 0x33, 0x65, 0xF2, 0xD3,
					       0x4F, 0x5C, 0x50, 0xF5, 0xFF, 0x84, 0xE1, 0xA2, 0xF9, 0xFF, 0x9D,
					       0x54, 0x0A, 0xEE, 0x76, 0x3C, 0x64, 0xB2, 0x11, 0xA0, 0xE8 };

const uint8_t sTestMsg[] = { "Hello Aliro" };

constexpr Aliro::CryptoTypes::PrivateKey kTestPrivateKey{ 0xfd, 0xf7, 0x1a, 0x37, 0x14, 0xe0, 0x78, 0xc2,
							  0xc2, 0xfa, 0x90, 0x7a, 0xe9, 0xac, 0xf6, 0x24,
							  0xaa, 0x98, 0xad, 0xd7, 0xed, 0xf7, 0x50, 0x0e,
							  0x61, 0xcf, 0x8a, 0xf4, 0xcc, 0x5a, 0x70, 0xa9 };

constexpr bool kTestStorage{ true };

constexpr Aliro::CryptoTypes::KeyId kPrivateKeyId{ CONFIG_DOOR_LOCK_ALIRO_READER_STORAGE_PRIVATE_KEY_PSA_KEY_ID };

#ifdef CONFIG_NCS_ALIRO_BLE_UWB
constexpr Aliro::CryptoTypes::KeyId kGroupResolvingKeyId{
	CONFIG_DOOR_LOCK_ALIRO_READER_STORAGE_GROUP_RESOLVING_KEY_PSA_KEY_ID
};
#endif // CONFIG_NCS_ALIRO_BLE_UWB

// Arbitrary PSA key ID, distinct from the reader private/group resolving
// key IDs, used to preserve a key.
constexpr Aliro::CryptoTypes::KeyId kKpersistentKeyId{ 0x40002 };
constexpr Aliro::CryptoTypes::KeyId kNonExistentKeyId{ 4 };

void GenerateRandomPublicKey(Aliro::CryptoTypes::PublicKey &publicKey)
{
	Aliro::CryptoTypes::PrivateKey privateKey{};
	auto ec = Aliro::Interface::Crypto::GenerateRandom(privateKey.data(), privateKey.size());
	zassert_equal(ec, ALIRO_NO_ERROR, "Cannot generate random private key");

	Aliro::CryptoTypes::KeyId keyId = 0;
	ec = DoorLock::CryptoUtils::ImportPrivateKey(privateKey, false, keyId);
	zassert_equal(ec, ALIRO_NO_ERROR, "Cannot import private key");

	ec = DoorLock::CryptoUtils::ExportPublicKey(keyId, publicKey);
	zassert_equal(ec, ALIRO_NO_ERROR, "Cannot export public key");

	ec = Aliro::Interface::Crypto::DestroyKey(keyId);
	zassert_equal(ec, ALIRO_NO_ERROR, "Cannot destroy private key");
}

} // namespace

ZTEST(interface_impl_crypto_tests, test_Interface_GenerateRandom)
{
	std::array<uint8_t, 16> randomValue{};
	std::array<uint8_t, 16> tmpRandomValue{};

	for (int it = 0; it < 25; it++) {
		zassert_true(randomValue == tmpRandomValue);
		// Generate random bytes.
		zassert_equal(ALIRO_NO_ERROR,
			      Aliro::Interface::Crypto::GenerateRandom(randomValue.data(), randomValue.size()));
		// Verify whether previous data are not the same.
		zassert_false(randomValue == tmpRandomValue);
		// Store data for comparison.
		tmpRandomValue = randomValue;
	}
}

ZTEST(interface_impl_crypto_tests, test_Interface_GenerateEphemeralKeyPair)
{
	Aliro::CryptoTypes::KeyId keyId{ 0 };
	Aliro::CryptoTypes::PublicKey ephemeralPublicKey{};

	auto ec = Aliro::Interface::Crypto::GenerateEphemeralKeyPair(keyId, ephemeralPublicKey);
	zassert_equal(ec, ALIRO_NO_ERROR, "Cannot generate ephemeral key pair");
	zassert_not_equal(keyId, 0, "Key ID is not set");
	zassert_not_equal(ephemeralPublicKey, Aliro::CryptoTypes::PublicKey{}, "Ephemeral public key is not set");

	ec = Aliro::Interface::Crypto::DestroyKey(keyId);
	zassert_equal(ec, ALIRO_NO_ERROR, "Cannot destroy ephemeral key pair");
}

ZTEST(interface_impl_crypto_tests, test_Interface_ImportSharedKey)
{
	Aliro::CryptoTypes::KeyId keyId{ 0 };
	std::array<uint8_t, 32> sharedKey{};

	auto ec = Aliro::Interface::Crypto::GenerateRandom(sharedKey.data(), sharedKey.size());
	zassert_equal(ec, ALIRO_NO_ERROR, "Cannot generate random shared key");

	ec = Aliro::Interface::Crypto::ImportSharedKey(sharedKey.data(), sharedKey.size(), keyId);
	zassert_equal(ec, ALIRO_NO_ERROR, "Cannot import shared key");
	zassert_not_equal(keyId, 0, "Key ID is not set");

	ec = Aliro::Interface::Crypto::DestroyKey(keyId);
	zassert_equal(ec, ALIRO_NO_ERROR, "Cannot destroy shared key");
}

ZTEST(interface_impl_crypto_tests, test_Interface_ImportSymmetricKey)
{
	Aliro::CryptoTypes::KeyId keyId{ 0 };
	std::array<uint8_t, 32> symmetricKey{};

	auto ec = Aliro::Interface::Crypto::GenerateRandom(symmetricKey.data(), symmetricKey.size());
	zassert_equal(ec, ALIRO_NO_ERROR, "Cannot generate random symmetric key");

	ec = Aliro::Interface::Crypto::ImportSymmetricKey(symmetricKey.data(), symmetricKey.size(), keyId);
	zassert_equal(ec, ALIRO_NO_ERROR, "Cannot import symmetric key");
	zassert_not_equal(keyId, 0, "Key ID is not set");

	ec = Aliro::Interface::Crypto::DestroyKey(keyId);
	zassert_equal(ec, ALIRO_NO_ERROR, "Cannot destroy symmetric key");
}

ZTEST(interface_impl_crypto_tests, test_Interface_DestroyKey)
{
	{
		Aliro::CryptoTypes::KeyId keyId{ 0 };
		const auto ec = Aliro::Interface::Crypto::DestroyKey(keyId);
		zassert_equal(ec, ALIRO_NO_ERROR, "Cannot destroy key");
		zassert_equal(keyId, 0, "Key ID is not set to 0");
	}

	{
		Aliro::CryptoTypes::KeyId keyId{ kNonExistentKeyId };
		const auto ec = Aliro::Interface::Crypto::DestroyKey(keyId);
		zassert_equal(ec, ALIRO_ERROR_INTERNAL, "Expected error when destroying non-existent key");
		zassert_equal(keyId, kNonExistentKeyId, "Key ID is not set to non-existent key ID");
	}

	{
		Aliro::CryptoTypes::KeyId keyId{ 0 };
		std::array<uint8_t, 32> symmetricKey{};

		auto ec = Aliro::Interface::Crypto::ImportSymmetricKey(symmetricKey.data(), symmetricKey.size(), keyId);
		zassert_equal(ec, ALIRO_NO_ERROR, "Cannot import symmetric key");
		zassert_not_equal(keyId, 0, "Key ID is not set");

		ec = Aliro::Interface::Crypto::DestroyKey(keyId);
		zassert_equal(ec, ALIRO_NO_ERROR, "Cannot destroy symmetric key");
		zassert_equal(keyId, 0, "Key ID is not set to 0");
	}
}

ZTEST(interface_impl_crypto_tests, test_Interface_GenerateSignature_ExportPublicKey_VerifySignature)
{
	std::array<uint8_t, 32> message{};

	// Generate random message.
	auto ec = Aliro::Interface::Crypto::GenerateRandom(message.data(), message.size());
	zassert_equal(ec, ALIRO_NO_ERROR, "Cannot generate random message");

	// Import private key.
	Aliro::CryptoTypes::KeyId keyId{ kPrivateKeyId };
	ec = DoorLock::CryptoUtils::ImportPrivateKey(kTestPrivateKey, kTestStorage, keyId);
	zassert_equal(ec, ALIRO_NO_ERROR, "Cannot import private key");

	// Generate signature.
	Aliro::CryptoTypes::Signature signature{};
	ec = Aliro::Interface::Crypto::GenerateSignature(message.data(), message.size(), signature);
	zassert_equal(ec, ALIRO_NO_ERROR, "Cannot generate signature");
	zassert_not_equal(signature, Aliro::CryptoTypes::Signature{}, "Signature is not set");

	// Export public key.
	Aliro::CryptoTypes::PublicKey pubKey{};
	ec = DoorLock::CryptoUtils::ExportPublicKey(keyId, pubKey);
	zassert_equal(ec, ALIRO_NO_ERROR, "Cannot export public key");

	// Verify signature.
	{
		ec = Aliro::Interface::Crypto::VerifySignature(pubKey, message.data(), message.size(), signature);
		zassert_equal(ec, ALIRO_NO_ERROR, "Cannot verify signature");
	}

	// Verify signature with KeyId
	{
		Aliro::CryptoTypes::PublicKey exportedPubKey{};
		ec = DoorLock::CryptoUtils::ExportPublicKey(keyId, exportedPubKey);
		zassert_equal(ec, ALIRO_NO_ERROR, "Cannot export public key");

		Aliro::CryptoTypes::KeyId pubKeyId{ 0 };
		ec = DoorLock::CryptoUtils::ImportPublicKey(exportedPubKey, false, pubKeyId);
		zassert_equal(ec, ALIRO_NO_ERROR, "Cannot import public key");
		zassert_not_equal(pubKeyId, 0, "Key ID is not set");

		ec = DoorLock::CryptoUtils::VerifySignature(pubKeyId, message.data(), message.size(), signature);
		zassert_equal(ec, ALIRO_NO_ERROR, "Cannot verify signature");

		ec = DoorLock::CryptoUtils::DestroyKey(pubKeyId);
		zassert_equal(ec, ALIRO_NO_ERROR, "Cannot destroy public key");
	}

	// Verify signature with invalid public key.
	{
		Aliro::CryptoTypes::PublicKey randomPubKey{};
		GenerateRandomPublicKey(randomPubKey);

		ec = Aliro::Interface::Crypto::VerifySignature(randomPubKey, message.data(), message.size(), signature);
		zassert_equal(ec, ALIRO_INVALID_SIGNATURE, "Expected invalid signature");
	}

	// Verify signature with invalid signature.
	{
		Aliro::CryptoTypes::Signature invalidSignature{};
		ec = Aliro::Interface::Crypto::GenerateRandom(invalidSignature.data(), invalidSignature.size());
		zassert_equal(ec, ALIRO_NO_ERROR, "Cannot generate random signature");

		ec = Aliro::Interface::Crypto::VerifySignature(pubKey, message.data(), message.size(),
							       invalidSignature);
		zassert_equal(ec, ALIRO_INVALID_SIGNATURE, "Expected invalid signature");
	}

	ec = Aliro::Interface::Crypto::DestroyKey(keyId);
	zassert_equal(ec, ALIRO_NO_ERROR, "Cannot destroy private key");
}

ZTEST(interface_impl_crypto_tests, test_Interface_RawKeyAgreement)
{
	Aliro::CryptoTypes::KeyId keyIdA{ 0 };
	Aliro::CryptoTypes::PublicKey publicKeyA{};

	auto ec = Aliro::Interface::Crypto::GenerateEphemeralKeyPair(keyIdA, publicKeyA);
	zassert_equal(ec, ALIRO_NO_ERROR, "Cannot generate ephemeral key pair");

	Aliro::CryptoTypes::KeyId keyIdB{ 0 };
	Aliro::CryptoTypes::PublicKey publicKeyB{};

	ec = Aliro::Interface::Crypto::GenerateEphemeralKeyPair(keyIdB, publicKeyB);
	zassert_equal(ec, ALIRO_NO_ERROR, "Cannot generate ephemeral key pair");

	Aliro::CryptoTypes::SharedSecret sharedSecretA{};
	ec = Aliro::Interface::Crypto::RawKeyAgreement(keyIdA, publicKeyB, sharedSecretA);
	zassert_equal(ec, ALIRO_NO_ERROR, "Cannot perform raw key agreement");
	zassert_not_equal(sharedSecretA, Aliro::CryptoTypes::SharedSecret{}, "Shared secret is not set");

	Aliro::CryptoTypes::SharedSecret sharedSecretB{};
	ec = Aliro::Interface::Crypto::RawKeyAgreement(keyIdB, publicKeyA, sharedSecretB);
	zassert_equal(ec, ALIRO_NO_ERROR, "Cannot perform raw key agreement");
	zassert_not_equal(sharedSecretB, Aliro::CryptoTypes::SharedSecret{}, "Shared secret is not set");

	zassert_true(sharedSecretA == sharedSecretB, "Shared secrets are not equal");

	ec = Aliro::Interface::Crypto::DestroyKey(keyIdA);
	zassert_equal(ec, ALIRO_NO_ERROR, "Cannot destroy ephemeral key pair");

	ec = Aliro::Interface::Crypto::DestroyKey(keyIdB);
	zassert_equal(ec, ALIRO_NO_ERROR, "Cannot destroy ephemeral key pair");
}

ZTEST(interface_impl_crypto_tests, test_Interface_DeriveSharedKey_DeriveSymmetricKey_DeriveRawKey)
{
	Aliro::CryptoTypes::KeyId sharedKeyId{ 0 };
	std::array<uint8_t, 32> sharedKey{};

	auto ec = Aliro::Interface::Crypto::GenerateRandom(sharedKey.data(), sharedKey.size());
	zassert_equal(ec, ALIRO_NO_ERROR, "Cannot generate random shared key");

	ec = Aliro::Interface::Crypto::ImportSharedKey(sharedKey.data(), sharedKey.size(), sharedKeyId);
	zassert_equal(ec, ALIRO_NO_ERROR, "Cannot import shared key");
	zassert_not_equal(sharedKeyId, 0, "Key ID is not set");

	{
		Aliro::CryptoTypes::KeyId derivedKeyId{ 0 };
		ec = Aliro::Interface::Crypto::DeriveSharedKey(sharedKeyId, sTestInfo.data(), sTestInfo.size(),
							       sTestSalt.data(), sTestSalt.size(), derivedKeyId);
		zassert_equal(ec, ALIRO_NO_ERROR, "Cannot derive shared key");
		zassert_not_equal(derivedKeyId, 0, "Derived key ID is not set");

		ec = Aliro::Interface::Crypto::DestroyKey(derivedKeyId);
		zassert_equal(ec, ALIRO_NO_ERROR, "Cannot destroy derived key");
	}

	{
		Aliro::CryptoTypes::KeyId symmetricKeyId{ 0 };
		ec = Aliro::Interface::Crypto::DeriveSymmetricKey(sharedKeyId, sTestInfo.data(), sTestInfo.size(),
								  sTestSalt.data(), sTestSalt.size(), symmetricKeyId);
		zassert_equal(ec, ALIRO_NO_ERROR, "Cannot derive symmetric key");
		zassert_not_equal(symmetricKeyId, 0, "Derived key ID is not set");

		ec = Aliro::Interface::Crypto::DestroyKey(symmetricKeyId);
		zassert_equal(ec, ALIRO_NO_ERROR, "Cannot destroy symmetric key");
	}

	{
		using RawKey = std::array<uint8_t, 160>;
		RawKey rawKey{};

		ec = Aliro::Interface::Crypto::DeriveRawKey(sharedKeyId, sTestInfo.data(), sTestInfo.size(),
							    sTestSalt.data(), sTestSalt.size(), rawKey.data(),
							    rawKey.size());
		zassert_equal(ec, ALIRO_NO_ERROR, "Cannot derive raw key");
		zassert_not_equal(rawKey, RawKey{}, "Raw key is not set");
	}

	ec = Aliro::Interface::Crypto::DestroyKey(sharedKeyId);
	zassert_equal(ec, ALIRO_NO_ERROR, "Cannot destroy shared key");
}

ZTEST(interface_impl_crypto_tests, test_Interface_AeadEncrypt_AeadDecrypt)
{
	Aliro::CryptoTypes::KeyId keyId{ 0 };
	std::array<uint8_t, 32> symmetricKey{};
	auto ec = Aliro::Interface::Crypto::GenerateRandom(symmetricKey.data(), symmetricKey.size());
	zassert_equal(ec, ALIRO_NO_ERROR, "Cannot generate random symmetric key");

	ec = Aliro::Interface::Crypto::ImportSymmetricKey(symmetricKey.data(), symmetricKey.size(), keyId);
	zassert_equal(ec, ALIRO_NO_ERROR, "Cannot import symmetric key");
	zassert_not_equal(keyId, 0, "Key ID is not set");

	std::array<uint8_t, 4> additionalData{};
	ec = Aliro::Interface::Crypto::GenerateRandom(additionalData.data(), additionalData.size());
	zassert_equal(ec, ALIRO_NO_ERROR, "Cannot generate random additional data");

	Aliro::CryptoTypes::Nonce nonce{};
	ec = Aliro::Interface::Crypto::GenerateRandom(nonce.data(), nonce.size());
	zassert_equal(ec, ALIRO_NO_ERROR, "Cannot generate random nonce");

	Aliro::CryptoTypes::AuthenticationTag authTag{};
	std::array<uint8_t, sizeof(sTestMsg)> cipherText{};
	ec = Aliro::Interface::Crypto::AeadEncrypt(keyId, sTestMsg, sizeof(sTestMsg), additionalData.data(),
						   additionalData.size(), nonce, cipherText.data(), authTag);
	zassert_equal(ec, ALIRO_NO_ERROR, "Cannot encrypt");
	zassert_not_equal(authTag, Aliro::CryptoTypes::AuthenticationTag{}, "Authentication tag is not set");

	std::array<uint8_t, sizeof(sTestMsg) + Aliro::CryptoTypes::kAuthenticationTagLength> cipherTextWithTag{};
	std::copy(cipherText.begin(), cipherText.end(), cipherTextWithTag.begin());
	std::copy(authTag.begin(), authTag.end(), cipherTextWithTag.begin() + cipherText.size());

	std::array<uint8_t, sizeof(sTestMsg)> plainText{};
	size_t plainTextLength = plainText.size();
	ec = Aliro::Interface::Crypto::AeadDecrypt(keyId, cipherTextWithTag.data(), cipherTextWithTag.size(),
						   additionalData.data(), additionalData.size(), nonce,
						   plainText.data(), plainTextLength);
	zassert_equal(ec, ALIRO_NO_ERROR, "Cannot decrypt");
	zassert_equal(plainTextLength, sizeof(sTestMsg), "Incorrect plaintext length");
	zassert_true(std::equal(sTestMsg, sTestMsg + sizeof(sTestMsg), plainText.begin()),
		     "Decrypted payload does not match the original one");

	ec = Aliro::Interface::Crypto::DestroyKey(keyId);
	zassert_equal(ec, ALIRO_NO_ERROR, "Cannot destroy symmetric key");
}

#ifdef CONFIG_NCS_ALIRO_BLE_UWB

ZTEST(interface_impl_crypto_tests, test_Interface_Encrypt)
{
	Aliro::CryptoTypes::GroupResolvingKey groupResolvingKey{};
	auto ec = Aliro::Interface::Crypto::GenerateRandom(groupResolvingKey.data(), groupResolvingKey.size());
	zassert_equal(ec, ALIRO_NO_ERROR, "Cannot generate random group resolving key");

	Aliro::CryptoTypes::KeyId keyId{ kGroupResolvingKeyId };
	ec = DoorLock::CryptoUtils::ImportGroupResolvingKey(groupResolvingKey, kTestStorage, keyId);
	zassert_equal(ec, ALIRO_NO_ERROR, "Cannot import group resolving key");
	zassert_not_equal(keyId, 0, "Key ID is not set");

	using Buffer = std::array<uint8_t, 16>;

	Buffer testPayload{};
	zassert_equal(ALIRO_NO_ERROR, Aliro::Interface::Crypto::GenerateRandom(testPayload.data(), testPayload.size()));

	Buffer cipherText{};
	ec = Aliro::Interface::Crypto::Encrypt(testPayload.data(), testPayload.size(), cipherText.data());
	zassert_equal(ec, ALIRO_NO_ERROR, "Cannot encrypt");

	Buffer plainText{};
	size_t plainTextLength = plainText.size();

	const auto status = psa_cipher_decrypt(keyId, PSA_ALG_ECB_NO_PADDING, cipherText.data(), cipherText.size(),
					       plainText.data(), plainText.size(), &plainTextLength);
	zassert_equal(status, PSA_SUCCESS, "Cannot decrypt");
	zassert_equal(plainTextLength, testPayload.size(), "Incorrect plaintext length");
	zassert_equal(testPayload, plainText, "Decrypted payload does not match the original one");

	ec = Aliro::Interface::Crypto::DestroyKey(keyId);
	zassert_equal(ec, ALIRO_NO_ERROR, "Cannot destroy group resolving key");
}

#endif // CONFIG_NCS_ALIRO_BLE_UWB

ZTEST(interface_impl_crypto_tests, test_Interface_Sha256)
{
	Aliro::CryptoTypes::Sha256Hash hash{};
	const auto ec = Aliro::Interface::Crypto::Sha256(sTestMsg, sizeof(sTestMsg), hash);
	zassert_equal(ec, ALIRO_NO_ERROR, "Cannot compute SHA-256 hash");

	constexpr Aliro::CryptoTypes::Sha256Hash kExpectedHash{ 0xcf, 0x1d, 0x52, 0xc1, 0x84, 0xc8, 0xd4, 0x8c,
								0xe0, 0x8b, 0xf5, 0x42, 0x4c, 0x33, 0xd9, 0xb0,
								0xe5, 0xe8, 0x9a, 0x80, 0xcb, 0xec, 0xe2, 0x3b,
								0xab, 0xdf, 0x0e, 0xb7, 0x62, 0xf3, 0x49, 0xde };
	zassert_equal(hash, kExpectedHash, "SHA-256 hash is not correct");
}

ZTEST(interface_impl_crypto_tests, test_Crypto_ImportPublicKey_ExportPublicKey)
{
	Aliro::CryptoTypes::PublicKey publicKey{};
	GenerateRandomPublicKey(publicKey);

	Aliro::CryptoTypes::KeyId keyId{ 0 };
	auto ec = ::DoorLock::CryptoUtils::ImportPublicKey(publicKey, false, keyId);
	zassert_equal(ec, ALIRO_NO_ERROR, "Cannot import public key");
	zassert_not_equal(keyId, 0, "Key ID is not set");

	Aliro::CryptoTypes::PublicKey exportedPublicKey{};
	ec = ::DoorLock::CryptoUtils::ExportPublicKey(keyId, exportedPublicKey);
	zassert_equal(ec, ALIRO_NO_ERROR, "Cannot export public key");

	zassert_equal(publicKey, exportedPublicKey, "Exported public key does not match the original one");

	ec = Aliro::Interface::Crypto::DestroyKey(keyId);
	zassert_equal(ec, ALIRO_NO_ERROR, "Cannot destroy public key");
}

ZTEST(interface_impl_crypto_tests, test_Crypto_PreserveKey_IsKeyAvailable)
{
	Aliro::CryptoTypes::KeyId keyId{ 0 };
	std::array<uint8_t, 32> sharedKey{};

	auto ec = Aliro::Interface::Crypto::GenerateRandom(sharedKey.data(), sharedKey.size());
	zassert_equal(ec, ALIRO_NO_ERROR, "Cannot generate random shared key");

	ec = DoorLock::CryptoUtils::IsKeyAvailable(keyId);
	zassert_not_equal(ec, ALIRO_NO_ERROR, "Expected invalid key");

	ec = Aliro::Interface::Crypto::ImportSharedKey(sharedKey.data(), sharedKey.size(), keyId);
	zassert_equal(ec, ALIRO_NO_ERROR, "Cannot import shared key");
	zassert_not_equal(keyId, 0, "Key ID is not set");

	ec = DoorLock::CryptoUtils::IsKeyAvailable(keyId);
	zassert_equal(ec, ALIRO_NO_ERROR, "Expected valid key");

	const auto keyIdCopy{ keyId };

	Aliro::CryptoTypes::KeyId persistentKeyId{ kKpersistentKeyId };
	ec = DoorLock::CryptoUtils::PreserveKey(keyId, persistentKeyId);
	zassert_equal(ec, ALIRO_NO_ERROR, "Cannot preserve key");

	ec = DoorLock::CryptoUtils::IsKeyAvailable(keyId);
	zassert_equal(ec, ALIRO_NO_ERROR, "Expected valid key");

	ec = DoorLock::CryptoUtils::DestroyKey(keyId);
	zassert_equal(ec, ALIRO_NO_ERROR, "Cannot destroy shared key");

	ec = DoorLock::CryptoUtils::IsKeyAvailable(persistentKeyId);
	zassert_equal(ec, ALIRO_NO_ERROR, "Expected valid key");

	ec = DoorLock::CryptoUtils::IsKeyAvailable(keyIdCopy);
	zassert_not_equal(ec, ALIRO_NO_ERROR, "Expected invalid key");

	ec = Aliro::Interface::Crypto::DestroyKey(persistentKeyId);
	zassert_equal(ec, ALIRO_NO_ERROR, "Cannot destroy shared key");
}

ZTEST(interface_impl_crypto_tests, test_Crypto_IsKeyAvailable)
{
	Aliro::CryptoTypes::KeyId keyId{ 0 };
	std::array<uint8_t, 32> sharedKey{};

	auto ec = Aliro::Interface::Crypto::GenerateRandom(sharedKey.data(), sharedKey.size());
	zassert_equal(ec, ALIRO_NO_ERROR, "Cannot generate random shared key");

	ec = DoorLock::CryptoUtils::IsKeyAvailable(keyId);
	zassert_not_equal(ec, ALIRO_NO_ERROR, "Expected invalid key");

	ec = Aliro::Interface::Crypto::ImportSharedKey(sharedKey.data(), sharedKey.size(), keyId);
	zassert_equal(ec, ALIRO_NO_ERROR, "Cannot import shared key");
	zassert_not_equal(keyId, 0, "Key ID is not set");

	ec = DoorLock::CryptoUtils::IsKeyAvailable(keyId);
	zassert_equal(ec, ALIRO_NO_ERROR, "Expected valid key");

	const auto keyIdCopy{ keyId };

	ec = Aliro::Interface::Crypto::DestroyKey(keyId);
	zassert_equal(ec, ALIRO_NO_ERROR, "Cannot destroy shared key");

	ec = DoorLock::CryptoUtils::IsKeyAvailable(keyIdCopy);
	zassert_not_equal(ec, ALIRO_NO_ERROR, "Expected invalid key");
}

void *setup_suite(void)
{
	int status = settings_subsys_init();
	zassert_equal(status, 0, "Settings subsystem initialization failed");

	void *storage{ nullptr };
	status = settings_storage_get(&storage);
	zassert_equal(status, 0, "Cannot get storage\n");

	if (IS_ENABLED(CONFIG_SETTINGS_NVS)) {
		status = nvs_clear(static_cast<nvs_fs *>(storage));
		zassert_equal(status, 0, "Cannot clear storage\n");
		status = nvs_mount(static_cast<nvs_fs *>(storage));
		zassert_equal(status, 0, "Cannot mount storage\n");
	} else if (IS_ENABLED(CONFIG_SETTINGS_ZMS)) {
		status = zms_clear(static_cast<zms_fs *>(storage));
		zassert_equal(status, 0, "Cannot clear storage\n");
		status = zms_mount(static_cast<zms_fs *>(storage));
		zassert_equal(status, 0, "Cannot mount storage\n");
	}

	status = settings_subsys_init();
	zassert_equal(status, 0, "Settings subsystem initialization failed");

	const auto ec = ::DoorLock::CryptoUtils::Init();
	zassert_equal(ec, ALIRO_NO_ERROR, "Cannot initialize Aliro crypto");

	return nullptr;
}

ZTEST_SUITE(interface_impl_crypto_tests, nullptr, setup_suite, nullptr, nullptr, nullptr);
