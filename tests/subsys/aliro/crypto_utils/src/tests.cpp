/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "aliro/types.h"
#include <zephyr/ztest.h>

#include <crypto_utils/crypto_utils.h>
#include <psa/crypto.h>

namespace {

constexpr Aliro::CryptoTypes::PrivateKey kTestPrivateKey{ 0xfd, 0xf7, 0x1a, 0x37, 0x14, 0xe0, 0x78, 0xc2,
							  0xc2, 0xfa, 0x90, 0x7a, 0xe9, 0xac, 0xf6, 0x24,
							  0xaa, 0x98, 0xad, 0xd7, 0xed, 0xf7, 0x50, 0x0e,
							  0x61, 0xcf, 0x8a, 0xf4, 0xcc, 0x5a, 0x70, 0xa9 };

constexpr Aliro::CryptoTypes::KeyId kPrivateKeyId{ 1 };
constexpr Aliro::CryptoTypes::KeyId kGroupResolvingKeyId{ 2 };
constexpr Aliro::CryptoTypes::KeyId kKpersistentKeyId{ 3 };
constexpr Aliro::CryptoTypes::KeyId kNonExistentKeyId{ 4 };

template <typename T> void GenerateRandom(T &buffer)
{
	auto status = psa_generate_random(buffer.data(), buffer.size());
	zassert_equal(status, PSA_SUCCESS, "Cannot generate random buffer");
}

void GenerateRandomPublicKey(Aliro::CryptoTypes::PublicKey &publicKey)
{
	Aliro::CryptoTypes::PrivateKey privateKey{};
	GenerateRandom(privateKey);

	Aliro::CryptoTypes::KeyId keyId = 0;
	auto ec = DoorLock::CryptoUtils::ImportPrivateKey(privateKey, false, keyId);
	zassert_equal(ec, ALIRO_NO_ERROR, "Cannot import private key");

	ec = DoorLock::CryptoUtils::ExportPublicKey(keyId, publicKey);
	zassert_equal(ec, ALIRO_NO_ERROR, "Cannot export public key");

	ec = ::DoorLock::CryptoUtils::DestroyKey(keyId);
	zassert_equal(ec, ALIRO_NO_ERROR, "Cannot destroy private key");
}

template <typename T>
AliroError VerifySignature(Aliro::CryptoTypes::PublicKey &publicKey, T &message,
			   const Aliro::CryptoTypes::Signature &signature)
{
	Aliro::CryptoTypes::KeyId keyId{ 0 };
	auto ec = DoorLock::CryptoUtils::ImportPublicKey(publicKey, false, keyId);
	zassert_equal(ec, ALIRO_NO_ERROR, "Cannot import public key");
	zassert_not_equal(keyId, 0, "Key ID is not set");

	const auto status = DoorLock::CryptoUtils::VerifySignature(keyId, message.data(), message.size(), signature);

	ec = DoorLock::CryptoUtils::DestroyKey(keyId);
	zassert_equal(ec, ALIRO_NO_ERROR, "Cannot destroy public key");

	return status;
}

} // namespace

ZTEST(crypto_utils_tests, test_ImportPrivateKey)
{
	{
		Aliro::CryptoTypes::KeyId keyId{ 0 };
		auto ec = ::DoorLock::CryptoUtils::ImportPrivateKey(kTestPrivateKey, false, keyId);
		zassert_equal(ec, ALIRO_NO_ERROR, "Cannot import private key");

		ec = ::DoorLock::CryptoUtils::DestroyKey(keyId);
		zassert_equal(ec, ALIRO_NO_ERROR, "Cannot destroy private key");
	}

	{
		Aliro::CryptoTypes::KeyId keyId{ kPrivateKeyId };
		auto ec = ::DoorLock::CryptoUtils::ImportPrivateKey(kTestPrivateKey, true, keyId);
		zassert_equal(ec, ALIRO_NO_ERROR, "Cannot import private key");
		zassert_not_equal(keyId, 0, "Key ID is not set");

		ec = ::DoorLock::CryptoUtils::DestroyKey(keyId);
		zassert_equal(ec, ALIRO_NO_ERROR, "Cannot destroy private key");
	}
}

ZTEST(crypto_utils_tests, test_ImportPublicKey_ExportPublicKey)
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

	ec = ::DoorLock::CryptoUtils::DestroyKey(keyId);
	zassert_equal(ec, ALIRO_NO_ERROR, "Cannot destroy public key");
}

ZTEST(crypto_utils_tests, test_ImportGroupResolvingKey_ExportKey)
{
	Aliro::CryptoTypes::GroupResolvingKey testGroupResolvingKey{};
	GenerateRandom(testGroupResolvingKey);

	{
		Aliro::CryptoTypes::KeyId keyId{ 0 };
		auto ec = ::DoorLock::CryptoUtils::ImportGroupResolvingKey(testGroupResolvingKey, false, keyId);
		zassert_equal(ec, ALIRO_NO_ERROR, "Cannot import group resolving key");
		zassert_not_equal(keyId, 0, "Key ID is not set");

		Aliro::CryptoTypes::GroupResolvingKey exportedKey{};
		ec = ::DoorLock::CryptoUtils::ExportKey(keyId, exportedKey.data(), exportedKey.size());
		zassert_equal(ec, ALIRO_NO_ERROR, "Cannot export key");

		zassert_equal(testGroupResolvingKey, exportedKey, "Exported key does not match the original one");

		ec = ::DoorLock::CryptoUtils::DestroyKey(keyId);
		zassert_equal(ec, ALIRO_NO_ERROR, "Cannot destroy group resolving key");
	}

	{
		Aliro::CryptoTypes::KeyId keyId{ kGroupResolvingKeyId };
		auto ec = ::DoorLock::CryptoUtils::ImportGroupResolvingKey(testGroupResolvingKey, true, keyId);
		zassert_equal(ec, ALIRO_NO_ERROR, "Cannot import group resolving key");
		zassert_not_equal(keyId, 0, "Key ID is not set");

		Aliro::CryptoTypes::GroupResolvingKey exportedKey{};
		ec = ::DoorLock::CryptoUtils::ExportKey(keyId, exportedKey.data(), exportedKey.size());
		zassert_equal(ec, ALIRO_NO_ERROR, "Cannot export key");

		zassert_equal(testGroupResolvingKey, exportedKey, "Exported key does not match the original one");

		ec = ::DoorLock::CryptoUtils::DestroyKey(keyId);
		zassert_equal(ec, ALIRO_NO_ERROR, "Cannot destroy group resolving key");
	}
}

ZTEST(crypto_utils_tests, test_PreserveKey_IsKeyAvailable)
{
	psa_key_attributes_t attributes = PSA_KEY_ATTRIBUTES_INIT;

	psa_set_key_type(&attributes, PSA_KEY_TYPE_DERIVE);
	psa_set_key_algorithm(&attributes, PSA_ALG_HKDF(PSA_ALG_SHA_256));
	psa_set_key_bits(&attributes, PSA_BYTES_TO_BITS(Aliro::CryptoTypes::kEccP256KeyPrivateKeyLength));
	psa_set_key_usage_flags(&attributes, PSA_KEY_USAGE_DERIVE | PSA_KEY_USAGE_COPY);

	Aliro::CryptoTypes::KeyId keyId{ 0 };
	std::array<uint8_t, Aliro::CryptoTypes::kEccP256KeyPrivateKeyLength> sharedKey{};
	GenerateRandom(sharedKey);

	auto ec = DoorLock::CryptoUtils::IsKeyAvailable(keyId);
	zassert_not_equal(ec, ALIRO_NO_ERROR, "Expected invalid key");

	psa_status_t status = psa_import_key(&attributes, sharedKey.data(), sharedKey.size(), &keyId);
	zassert_equal(status, PSA_SUCCESS, "Cannot import shared key");
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

	ec = DoorLock::CryptoUtils::DestroyKey(persistentKeyId);
	zassert_equal(ec, ALIRO_NO_ERROR, "Cannot destroy shared key");
}

ZTEST(crypto_utils_tests, test_DestroyKey)
{
	{
		Aliro::CryptoTypes::KeyId keyId{ 0 };
		const auto ec = ::DoorLock::CryptoUtils::DestroyKey(keyId);
		zassert_equal(ec, ALIRO_NO_ERROR, "Cannot destroy key");
		zassert_equal(keyId, 0, "Key ID is not set to 0");
	}

	{
		Aliro::CryptoTypes::KeyId keyId{ kNonExistentKeyId };
		const auto ec = ::DoorLock::CryptoUtils::DestroyKey(keyId);
		zassert_equal(ec, ALIRO_ERROR_INTERNAL, "Expected error when destroying non-existent key");
		zassert_equal(keyId, kNonExistentKeyId, "Key ID is not set to non-existent key ID");
	}

	{
		Aliro::CryptoTypes::KeyId keyId{ 0 };

		auto ec = ::DoorLock::CryptoUtils::ImportPrivateKey(kTestPrivateKey, false, keyId);
		zassert_equal(ec, ALIRO_NO_ERROR, "Cannot import private key");
		zassert_not_equal(keyId, 0, "Key ID is not set");

		ec = ::DoorLock::CryptoUtils::DestroyKey(keyId);
		zassert_equal(ec, ALIRO_NO_ERROR, "Cannot destroy private key");
		zassert_equal(keyId, 0, "Key ID is not set to 0");
	}
}

ZTEST(crypto_utils_tests, test_VerifySignature)
{
	std::array<uint8_t, 32> message{};
	GenerateRandom(message);

	// Import private key.
	Aliro::CryptoTypes::KeyId keyId{ kPrivateKeyId };
	auto ec = DoorLock::CryptoUtils::ImportPrivateKey(kTestPrivateKey, false, keyId);
	zassert_equal(ec, ALIRO_NO_ERROR, "Cannot import private key");

	// Export public key.
	Aliro::CryptoTypes::PublicKey pubKey{};
	ec = DoorLock::CryptoUtils::ExportPublicKey(keyId, pubKey);
	zassert_equal(ec, ALIRO_NO_ERROR, "Cannot export public key");

	// Generate signature.
	Aliro::CryptoTypes::Signature signature{};

	size_t outputLen{};
	auto status = psa_sign_message(keyId, PSA_ALG_ECDSA(PSA_ALG_SHA_256), message.data(), message.size(),
				       signature.data(), signature.size(), &outputLen);
	zassert_equal(status, PSA_SUCCESS, "Cannot generate signature");
	zassert_equal(outputLen, signature.size(), "Invalid signature length");

	// Destroy private key.
	ec = DoorLock::CryptoUtils::DestroyKey(keyId);
	zassert_equal(ec, ALIRO_NO_ERROR, "Cannot destroy private key");

	// Verify signature with exported public key.
	{
		const auto ec = VerifySignature(pubKey, message, signature);
		zassert_equal(ec, ALIRO_NO_ERROR, "Cannot verify signature");
	}

	// Verify signature with random public key.
	{
		Aliro::CryptoTypes::PublicKey randomPubKey{};
		GenerateRandomPublicKey(randomPubKey);
		const auto ec = VerifySignature(randomPubKey, message, signature);
		zassert_equal(ec, ALIRO_INVALID_SIGNATURE, "Expected invalid signature");
	}

	// Verify signature with invalid signature.
	{
		Aliro::CryptoTypes::Signature invalidSignature{};
		GenerateRandom(invalidSignature);

		const auto ec = VerifySignature(pubKey, message, invalidSignature);
		zassert_equal(ec, ALIRO_INVALID_SIGNATURE, "Expected invalid signature");
	}
}

void *setup_suite(void)
{
	const auto ec = ::DoorLock::CryptoUtils::Init();
	zassert_equal(ec, ALIRO_NO_ERROR, "Cannot initialize Aliro crypto utils");

	return nullptr;
}

ZTEST_SUITE(crypto_utils_tests, nullptr, setup_suite, nullptr, nullptr, nullptr);
