/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "door_lock_delegate.h"

#include "lib/support/CodeUtils.h"
#include <algorithm>
#include <aliro/aliro.h>
#include <aliro/init.h>
#include <aliro/interface.h>
#include <aliro/storage/storage.h>
#include <aliro/storage/storage_keys.h>
#include <platform/CHIPDeviceLayer.h>

#include "aliro/crypto_key_ids.h"
#include "crypto/utils.h"

#include <zephyr/logging/log.h>

LOG_MODULE_DECLARE(app, CONFIG_CHIP_APP_LOG_LEVEL);

using namespace chip::app::Clusters::DoorLock;

namespace {

#ifdef CONFIG_DOOR_LOCK_USE_TEST_READER_IDENTIFIER

constexpr std::array<uint8_t, kAliroReaderGroupSubIdentifierSize> kTestGroupSubIdentifier{
	0x63, 0x20, 0x38, 0x36, 0x20, 0x31, 0x62, 0x20, 0x33, 0x39, 0x20, 0x31, 0x66, 0x20, 0x33, 0x34
};

#endif // CONFIG_DOOR_LOCK_USE_TEST_READER_IDENTIFIER

static_assert(sizeof(Aliro::CryptoTypes::PrivateKey) == kAliroSigningKeySize,
	      "Aliro::CryptoTypes::PrivateKey size mismatch");
static_assert(sizeof(Aliro::CryptoTypes::PublicKey) == kAliroReaderVerificationKeySize,
	      "Aliro::CryptoTypes::PublicKey size mismatch");
static_assert(sizeof(Aliro::Identifier) == kAliroReaderGroupIdentifierSize + kAliroReaderGroupSubIdentifierSize,
	      "Aliro::Identifier size mismatch");
static_assert(sizeof(Aliro::CryptoTypes::GroupResolvingKey) == kAliroGroupResolvingKeySize,
	      "Aliro::CryptoTypes::GroupResolvingKey size mismatch");
static_assert(sizeof(Aliro::ProtocolVersion) == kAliroProtocolVersionSize, "Aliro::ProtocolVersion size mismatch");

CHIP_ERROR EncodeProtocolVersion(size_t index, chip::MutableByteSpan &protocolVersion,
				 const Aliro::ProtocolVersion *versions, size_t versionCount)
{
	VerifyOrReturnError(versions != nullptr, CHIP_ERROR_INVALID_ARGUMENT, LOG_ERR("Versions list is nullptr"));

	if (index > versionCount - 1) {
		protocolVersion.reduce_size(0);
		return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
	}

	if (protocolVersion.size() != kAliroProtocolVersionSize) {
		protocolVersion.reduce_size(0);
		return CHIP_ERROR_INVALID_ARGUMENT;
	}

	// Per Aliro spec, protocol version encoding is big-endian
	chip::Encoding::BigEndian::Put16(protocolVersion.data(), versions[index]);

	return CHIP_NO_ERROR;
}
} // namespace

CHIP_ERROR DoorLockDelegate::Init()
{
	CHIP_ERROR err = chip::DeviceLayer::SystemLayer().ScheduleLambda([]() {
		Aliro::CryptoTypes::PublicKey publicKey{};
		Aliro::Identifier identifier{};
		Aliro::CryptoTypes::KeyId groupResolvingKeyId{ 0 };

		AliroError ec = DoorLock::Crypto::ExportPublicKey(Aliro::kPrivateKeyId, publicKey);
		VerifyOrReturn(ec == ALIRO_NO_ERROR, /* device not provisioned */);

		VerifyOrReturn(KeyValueStorage::Instance().Get(Aliro::StorageKeys::kStorageKeyNameIdentifier,
							       identifier.data(), identifier.size()) == 0,
			       LOG_ERR("Failed to get reader group identifier"));

#ifdef CONFIG_DOOR_LOCK_BLE_UWB

		groupResolvingKeyId = Aliro::kGroupResolvingKeyId;

#endif // CONFIG_DOOR_LOCK_BLE_UWB

		ec = Aliro::AliroStack::Instance().Provision(Aliro::kPrivateKeyId, publicKey, groupResolvingKeyId,
							     identifier);
		VerifyOrReturn(ec == ALIRO_NO_ERROR, LOG_ERR("Failed to provision Aliro stack"));

		int err = AliroStart();
		if (err != EXIT_SUCCESS) {
			LOG_ERR("Failed to start Aliro");
		}
	});
	VerifyOrReturnError(err == CHIP_NO_ERROR, err, LOG_ERR("Failed to schedule lambda"));

	return CHIP_NO_ERROR;
}

CHIP_ERROR DoorLockDelegate::GetAliroReaderVerificationKey(chip::MutableByteSpan &verificationKey)
{
	LOG_DBG("GetAliroReaderVerificationKey");

	VerifyOrReturnError(verificationKey.size() == kAliroReaderVerificationKeySize, CHIP_ERROR_INVALID_ARGUMENT);

	Aliro::CryptoTypes::PublicKey publicKey{};
	AliroError ec = DoorLock::Crypto::ExportPublicKey(Aliro::kPrivateKeyId, publicKey);
	if (ec != ALIRO_NO_ERROR) {
		verificationKey.reduce_size(0);

		// We have to return CHIP_NO_ERROR here.
		return CHIP_NO_ERROR;
	}

	std::copy_n(publicKey.begin(), kAliroReaderVerificationKeySize, verificationKey.data());

	return CHIP_NO_ERROR;
}

CHIP_ERROR DoorLockDelegate::GetAliroReaderGroupIdentifier(chip::MutableByteSpan &groupIdentifier)
{
	LOG_DBG("GetAliroReaderGroupIdentifier");

	VerifyOrReturnError(groupIdentifier.size() == kAliroReaderGroupIdentifierSize, CHIP_ERROR_INVALID_ARGUMENT);

	Aliro::Identifier identifier{};

	if (KeyValueStorage::Instance().Get(Aliro::StorageKeys::kStorageKeyNameIdentifier, identifier.data(),
					    identifier.size()) != 0) {
		groupIdentifier.reduce_size(0);
		// We have to return CHIP_NO_ERROR here.
		return CHIP_NO_ERROR;
	}

	std::copy_n(identifier.data(), kAliroReaderGroupIdentifierSize, groupIdentifier.data());

	return CHIP_NO_ERROR;
}

CHIP_ERROR DoorLockDelegate::GetAliroReaderGroupSubIdentifier(chip::MutableByteSpan &groupSubIdentifier)
{
	LOG_DBG("GetAliroReaderGroupSubIdentifier");

	VerifyOrReturnError(groupSubIdentifier.size() == kAliroReaderGroupSubIdentifierSize,
			    CHIP_ERROR_INVALID_ARGUMENT);

	Aliro::Identifier identifier{};

	if (KeyValueStorage::Instance().Get(Aliro::StorageKeys::kStorageKeyNameIdentifier, identifier.data(),
					    identifier.size()) != 0) {
		groupSubIdentifier.reduce_size(0);
		// We have to return CHIP_NO_ERROR here.
		return CHIP_NO_ERROR;
	}

	std::copy_n(identifier.data() + kAliroReaderGroupIdentifierSize, kAliroReaderGroupSubIdentifierSize,
		    groupSubIdentifier.data());

	return CHIP_NO_ERROR;
}

CHIP_ERROR
DoorLockDelegate::GetAliroExpeditedTransactionSupportedProtocolVersionAtIndex(size_t index,
									      chip::MutableByteSpan &protocolVersion)
{
	LOG_DBG("GetAliroExpeditedTransactionSupportedProtocolVersionAtIndex");

	size_t versionCount{};
	const auto *versions = Aliro::AliroStack::Instance().GetExpeditedStandardProtocolVersions(versionCount);

	return EncodeProtocolVersion(index, protocolVersion, versions, versionCount);
}

CHIP_ERROR DoorLockDelegate::GetAliroGroupResolvingKey(chip::MutableByteSpan &groupResolvingKey)
{
	LOG_DBG("GetAliroGroupResolvingKey");

	VerifyOrReturnError(groupResolvingKey.size() == kAliroGroupResolvingKeySize, CHIP_ERROR_INVALID_ARGUMENT);

#ifdef CONFIG_DOOR_LOCK_BLE_UWB

	AliroError ec = DoorLock::Crypto::ExportKey(Aliro::kGroupResolvingKeyId, groupResolvingKey.data(),
						    groupResolvingKey.size());
	if (ec != ALIRO_NO_ERROR) {
		groupResolvingKey.reduce_size(0);
		return CHIP_ERROR_NOT_FOUND;
	}

	return CHIP_NO_ERROR;

#else // CONFIG_DOOR_LOCK_BLE_UWB

	groupResolvingKey.reduce_size(0);
	return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;

#endif // CONFIG_DOOR_LOCK_BLE_UWB
}

CHIP_ERROR DoorLockDelegate::GetAliroSupportedBLEUWBProtocolVersionAtIndex(size_t index,
									   chip::MutableByteSpan &protocolVersion)
{
	LOG_DBG("GetAliroSupportedBLEUWBProtocolVersionAtIndex");

#if CONFIG_DOOR_LOCK_BLE_UWB

	size_t versionCount{};
	const auto *versions = Aliro::AliroStack::Instance().GetBleUwbProtocolVersions(versionCount);

	return EncodeProtocolVersion(index, protocolVersion, versions, versionCount);

#else // CONFIG_DOOR_LOCK_BLE_UWB

	protocolVersion.reduce_size(0);
	return CHIP_ERROR_NOT_IMPLEMENTED;

#endif // CONFIG_DOOR_LOCK_BLE_UWB
}

uint8_t DoorLockDelegate::GetAliroBLEAdvertisingVersion()
{
	LOG_DBG("GetAliroBLEAdvertisingVersion");

#ifdef CONFIG_DOOR_LOCK_BLE_UWB

	return Aliro::AliroStack::Instance().GetBleAdvertisingVersion();

#else // CONFIG_DOOR_LOCK_BLE_UWB

	return 0;

#endif // CONFIG_DOOR_LOCK_BLE_UWB
}

uint16_t DoorLockDelegate::GetNumberOfAliroCredentialIssuerKeysSupported()
{
	LOG_DBG("GetNumberOfAliroCredentialIssuerKeysSupported");

	return CONFIG_DOOR_LOCK_ACCESS_MANAGER_CREDENTIAL_ISSUER_MAX_STORED_KEYS;
}

uint16_t DoorLockDelegate::GetNumberOfAliroEndpointKeysSupported()
{
	LOG_DBG("GetNumberOfAliroEndpointKeysSupported");

	return CONFIG_DOOR_LOCK_ACCESS_MANAGER_ACCESS_CREDENTIAL_MAX_STORED_KEYS;
}

CHIP_ERROR DoorLockDelegate::SetAliroReaderConfig(const chip::ByteSpan &signingKey,
						  const chip::ByteSpan &verificationKey,
						  const chip::ByteSpan &groupIdentifier,
						  const chip::Optional<chip::ByteSpan> &groupResolvingKey)
{
	LOG_DBG("SetAliroReaderConfig");

	VerifyOrReturnError(signingKey.size() == kAliroSigningKeySize, CHIP_ERROR_INVALID_ARGUMENT);
	VerifyOrReturnError(verificationKey.size() == kAliroReaderVerificationKeySize, CHIP_ERROR_INVALID_ARGUMENT);
	VerifyOrReturnError(groupIdentifier.size() == kAliroReaderGroupIdentifierSize, CHIP_ERROR_INVALID_ARGUMENT);

#ifdef CONFIG_DOOR_LOCK_BLE_UWB

	VerifyOrReturnError(groupResolvingKey.HasValue() &&
				    groupResolvingKey.Value().size() == kAliroGroupResolvingKeySize,
			    CHIP_ERROR_INVALID_ARGUMENT);

#endif // CONFIG_DOOR_LOCK_BLE_UWB

	Aliro::CryptoTypes::PrivateKey privateKey{};
	Aliro::Identifier identifier{};
	Aliro::CryptoTypes::GroupResolvingKey groupResKey{};
	Aliro::CryptoTypes::KeyId privateKeyId{ 0 };
	Aliro::CryptoTypes::PublicKey publicKey{};
	Aliro::CryptoTypes::KeyId groupResolvingKeyId{ 0 };

	std::copy(signingKey.begin(), signingKey.end(), privateKey.data());
	std::copy_n(groupIdentifier.data(), kAliroReaderGroupIdentifierSize, identifier.data());

#ifdef CONFIG_DOOR_LOCK_USE_TEST_READER_IDENTIFIER

	std::copy(kTestGroupSubIdentifier.begin(), kTestGroupSubIdentifier.end(),
		  identifier.data() + kAliroReaderGroupIdentifierSize);

#else

	AliroError err = Aliro::Interface::Crypto::GenerateRandom(identifier.data() + kAliroReaderGroupIdentifierSize,
								  kAliroReaderGroupSubIdentifierSize);
	VerifyOrReturnError(err == ALIRO_NO_ERROR, CHIP_ERROR_INTERNAL);

#endif // CONFIG_DOOR_LOCK_USE_TEST_READER_IDENTIFIER

	VerifyOrReturnError(KeyValueStorage::Instance().Save(Aliro::StorageKeys::kStorageKeyNameIdentifier,
							     identifier.data(), identifier.size()) == 0,
			    CHIP_ERROR_INTERNAL);

	if (groupResolvingKey.HasValue()) {
		std::copy(groupResolvingKey.Value().begin(), groupResolvingKey.Value().end(), groupResKey.data());
	}

	privateKeyId = Aliro::kPrivateKeyId;
	AliroError ec = DoorLock::Crypto::ImportPrivateKey(privateKey, true, privateKeyId);
	VerifyOrReturnError(ec == ALIRO_NO_ERROR, CHIP_ERROR_INTERNAL);

	ec = DoorLock::Crypto::ExportPublicKey(privateKeyId, publicKey);
	VerifyOrReturnError(ec == ALIRO_NO_ERROR, CHIP_ERROR_INTERNAL);

#ifdef CONFIG_DOOR_LOCK_BLE_UWB

	groupResolvingKeyId = Aliro::kGroupResolvingKeyId;
	ec = DoorLock::Crypto::ImportGroupResolvingKey(groupResKey, true, groupResolvingKeyId);
	VerifyOrReturnError(ec == ALIRO_NO_ERROR, CHIP_ERROR_INTERNAL);

#endif // CONFIG_DOOR_LOCK_BLE_UWB

	ec = Aliro::AliroStack::Instance().Provision(privateKeyId, publicKey, groupResolvingKeyId, identifier);
	VerifyOrReturnError(ec == ALIRO_NO_ERROR, CHIP_ERROR_INTERNAL, LOG_ERR("Failed to provision Aliro stack"));

	VerifyOrReturnError(AliroStart() == EXIT_SUCCESS, CHIP_ERROR_INTERNAL, LOG_ERR("Failed to start Aliro"););

	return CHIP_NO_ERROR;
}

CHIP_ERROR DoorLockDelegate::ClearAliroReaderConfig()
{
	LOG_DBG("ClearAliroReaderConfig");

	VerifyOrReturnError(AliroStop() == EXIT_SUCCESS, CHIP_ERROR_INTERNAL, LOG_ERR("Failed to stop Aliro"));

	KeyValueStorage::Instance().Clear(Aliro::StorageKeys::kStorageKeyNameIdentifier);

	Aliro::CryptoTypes::KeyId keyId{ Aliro::kPrivateKeyId };
	DoorLock::Crypto::DestroyKey(keyId);

#ifdef CONFIG_DOOR_LOCK_BLE_UWB

	keyId = Aliro::kGroupResolvingKeyId;
	DoorLock::Crypto::DestroyKey(keyId);

#endif // CONFIG_DOOR_LOCK_BLE_UWB

	return CHIP_NO_ERROR;
}
