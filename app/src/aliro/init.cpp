/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
#include "aliro/aliro.h"
#include "aliro/types.h"
#include "aliro/utils.h"
#include "crypto/crypto.h"

#ifdef CONFIG_ACCESS_DECISION_INDICATOR
#include "access_decision_indicator.h"
#endif // CONFIG_ACCESS_DECISION_INDICATOR

#ifdef CONFIG_DOOR_LOCK_USE_TEST_KEYS
#include "test_key.h"
#endif // CONFIG_DOOR_LOCK_USE_TEST_KEYS

#include "access_manager/access_manager.h"
#include "crypto_key_ids.h"
#include "shell.h"
#include "storage.h"
#include "storage_keys.h"

#include <zephyr/logging/log.h>
#include <zephyr/shell/shell.h>

#include <cstdio>
#include <stdlib.h>

LOG_MODULE_REGISTER(aliro, CONFIG_NCS_DOOR_LOCK_APP_LOG_LEVEL);

using namespace Aliro;
using namespace Aliro::Access;

namespace {

#ifndef CONFIG_CHIP

#ifdef CONFIG_DOOR_LOCK_USE_TEST_READER_IDENTIFIER

constexpr Identifier kTestIdentifier{ 0x37, 0x65, 0x20, 0x39, 0x31, 0x20, 0x61, 0x65, 0x20, 0x31, 0x64,
				      0x20, 0x33, 0x64, 0x20, 0x65, 0x63, 0x20, 0x38, 0x36, 0x20, 0x31,
				      0x62, 0x20, 0x33, 0x39, 0x20, 0x31, 0x66, 0x20, 0x33, 0x34 };

#endif // CONFIG_DOOR_LOCK_USE_TEST_READER_IDENTIFIER

AliroError LoadAccessCredentials()
{
	size_t keyCount{};

	for (size_t keyId = 0; keyId < CONFIG_ALIRO_ACCESS_MANAGER_MAX_STORED_KEYS; keyId++) {
		StorageKeys::KeyNameBuffer keyName;
		snprintf(keyName.data(), keyName.size(), "%s/%u", StorageKeys::kStorageKeyNameAccessCredentialPublicKey,
			 keyId);

		Aliro::CryptoTypes::PublicKey pubKey{};

		int ec = KeyValueStorage::Instance().Get(keyName.data(), pubKey.data(), pubKey.size());
		VerifyOrReturnStatus(ec == 0 || ec == -ENODATA, ALIRO_ERROR_INTERNAL,
				     LOG_ERR("Cannot get user device public key, error code: %d", ec));
		if (ec == 0) {
			AliroError err = AccessManagerInstance().AddPublicKey(pubKey);
			VerifyOrReturnStatus(err == ALIRO_NO_ERROR, err, LOG_ERR("Cannot set public key."));
			++keyCount;
		}
	}

	if (keyCount == 0) {
		LOG_INF("No Access Credential public keys available");
	}

	return ALIRO_NO_ERROR;
}

AliroError LoadReaderKeys(Aliro::CryptoTypes::KeyId &privateKeyId,
			  [[maybe_unused]] Aliro::CryptoTypes::KeyId &groupResolvingKeyId)
{
	Aliro::CryptoTypes::PrivateKey privateKey{};
	Aliro::CryptoTypes::PublicKey publicKey{};

	privateKeyId = Aliro::kPrivateKeyId;
	AliroError ec = Aliro::CryptoInstance().ExportPublicKey(privateKeyId, publicKey);
	if (ec != ALIRO_NO_ERROR) {
#ifdef CONFIG_DOOR_LOCK_USE_TEST_KEYS

		LOG_WRN("\n### WARNING: Tests keys are used (NOT allowed for production!) ###\n");
		privateKey = mPrivateKey;

#else /* CONFIG_DOOR_LOCK_USE_TEST_KEYS */

		LOG_DBG("\n### Production keys are used ###\n");
		return ALIRO_ERROR_NOT_IMPLEMENTED;

#endif /* CONFIG_DOOR_LOCK_USE_TEST_KEYS */

		AliroError err = Aliro::CryptoInstance().ImportPrivateKey(privateKey, privateKeyId, true);
		VerifyOrReturnStatus(err == ALIRO_NO_ERROR, err, LOG_ERR("Cannot import reader private key"));
	}

#ifdef CONFIG_ALIRO_BLE_TP

	groupResolvingKeyId = kGroupResolvingKeyId;

	Aliro::CryptoTypes::GroupResolvingKey groupResolvingKey{};
	ec = Aliro::CryptoInstance().ExportKey(Aliro::kGroupResolvingKeyId, groupResolvingKey.data(),
					       groupResolvingKey.size());
	if (ec != ALIRO_NO_ERROR) {
		LOG_DBG("Group Resolving Key is not provisioned, all-zero key will be used");
		AliroError err = Aliro::CryptoInstance().ProvisionSymmetricKey(
			groupResolvingKey.data(), groupResolvingKey.size(), groupResolvingKeyId, true);
		VerifyOrReturnStatus(err == ALIRO_NO_ERROR, err, LOG_ERR("Cannot provision group resolving key"));
	}

#endif // CONFIG_ALIRO_BLE_TP

	return ALIRO_NO_ERROR;
}

AliroError LoadReaderIdentifier(Identifier &identifier)
{
	int ec = KeyValueStorage::Instance().Get(StorageKeys::kStorageKeyNameIdentifier, identifier.data(),
						 identifier.size());
	[[maybe_unused]] bool identifierAvailable = ec == 0;

	if (ec == -ENODATA) {
#ifdef CONFIG_DOOR_LOCK_USE_TEST_READER_IDENTIFIER
		identifier = kTestIdentifier;
		ec = KeyValueStorage::Instance().Save(StorageKeys::kStorageKeyNameIdentifier, identifier.data(),
						      identifier.size());
		VerifyOrReturnStatus(ec == 0, ALIRO_ERROR_INTERNAL,
				     LOG_ERR("Cannot save reader identifier, error: %d", ec));
		identifierAvailable = true;
#else // CONFIG_DOOR_LOCK_USE_TEST_READER_IDENTIFIER
		LOG_INF("No reader identifier available");
#endif // CONFIG_DOOR_LOCK_USE_TEST_READER_IDENTIFIER
	} else if (ec) {
		LOG_ERR("Cannot get reader identifier, error code: %d", ec);
		return ALIRO_ERROR_INTERNAL;
	}

#ifdef CONFIG_DOOR_LOCK_PRINT_READER_GROUP_IDENTIFIER
	if (identifierAvailable) {
		// First 16 bytes of the Reader Identifier constitute the Reader Group Identifier
		char hexString[kReaderGroupIdentifierLength * 2 + 1];
		size_t resLen = bin2hex(identifier.data(), kReaderGroupIdentifierLength, hexString, sizeof(hexString));
		VerifyOrReturnStatus(resLen == kReaderGroupIdentifierLength * 2, ALIRO_ERROR_INTERNAL,
				     LOG_ERR("Cannot convert buffer to hex string"));

		LOG_INF("\nProvision the Test Harness with the following Reader Group Identifier:");
		LOG_INF("%s\n", hexString);
	}
#endif

	return ALIRO_NO_ERROR;
}

AliroError StorageInit()
{
	AliroError err = LoadAccessCredentials();
	VerifyOrReturnStatus(err == ALIRO_NO_ERROR, err, LOG_ERR("Cannot load Access Credentials"));

	Aliro::CryptoTypes::KeyId privateKeyId{ 0 };
	Aliro::CryptoTypes::KeyId groupResolvingKeyId{ 0 };
	err = LoadReaderKeys(privateKeyId, groupResolvingKeyId);
	VerifyOrReturnStatus(err == ALIRO_NO_ERROR, err, LOG_ERR("Cannot load reader keys"));

	Identifier identifier{};
	err = LoadReaderIdentifier(identifier);
	VerifyOrReturnStatus(err == ALIRO_NO_ERROR, err, LOG_ERR("Cannot load reader identifier"));

	err = AliroStack::Instance().Provision(privateKeyId, groupResolvingKeyId, identifier);
	VerifyOrReturnStatus(err == ALIRO_NO_ERROR, err, LOG_ERR("Cannot provision Aliro stack"));

	return ALIRO_NO_ERROR;
}

#endif // CONFIG_CHIP

} // namespace

int AliroInit()
{
	AliroError ec{};
	LOG_INF("Starting nRF Door Lock Reference Application for the nRF Connect SDK");

#ifdef CONFIG_ACCESS_DECISION_INDICATOR
	VerifyOrReturnValue(Indicator::InitAccessDecisionIndicator() == ALIRO_NO_ERROR, EXIT_FAILURE,
			    LOG_ERR("Failed to initialize access decision indicator"));
#endif // CONFIG_ACCESS_DECISION_INDICATOR

	AccessManagerInstance().Init({ .mAccessGrantedIndicatorClb = Indicator::SignalAccessGranted });

	const AliroConfig config{
#ifdef CONFIG_DISABLE_ALIRO_NFC_TP
		.mEnableNfc = false,
#endif // CONFIG_DISABLE_ALIRO_NFC_TP

#ifdef CONFIG_ALIRO_BLE_TP
		.mMaxBleSessions = CONFIG_ALIRO_BLE_TP_MAX_SESSIONS,
#endif // CONFIG_ALIRO_BLE_TP
	};

	ec = AliroStack::Instance().Init(
		{ .mOnError = [](AliroError error) { LOG_ERR("Aliro error: %s", error.ToString()); } }, config);

	VerifyOrReturnValue(ec == ALIRO_NO_ERROR, EXIT_FAILURE, LOG_ERR("Aliro stack initialization failed"));

#ifndef CONFIG_CHIP
	ec = StorageInit();
	VerifyOrReturnValue(ec == ALIRO_NO_ERROR, EXIT_FAILURE, LOG_ERR("Storage initialization failed"));

	ec = AliroStack::Instance().Start();
	VerifyOrReturnValue(ec == ALIRO_NO_ERROR, EXIT_FAILURE, LOG_ERR("Aliro stack start failed"));
#endif // CONFIG_CHIP

	RegisterShellCommands();

	LOG_INF("Aliro stack initialized");

	return EXIT_SUCCESS;
}

int AliroStart()
{
	AliroError ec = AliroStack::Instance().Start();

	VerifyOrReturnValue(ec == ALIRO_NO_ERROR, EXIT_FAILURE, LOG_ERR("Aliro stack start failed"));

	return EXIT_SUCCESS;
}

int AliroStop()
{
	AliroError ec = AliroStack::Instance().Stop();

	VerifyOrReturnValue(ec == ALIRO_NO_ERROR, EXIT_FAILURE, LOG_ERR("Aliro stack stop failed"));

	return EXIT_SUCCESS;
}

#ifdef CONFIG_CHIP

void ClearStorageAliro()
{
	int ec = KeyValueStorage::Instance().Clear(StorageKeys::kStorageKeyNameIdentifier);
	if (!ec) {
		LOG_ERR("Failed to clear %s/%s storage key: %d", StorageKeys::kDoorLockBaseKey,
			StorageKeys::kStorageKeyNameIdentifier, ec);
	}

	Aliro::CryptoTypes::KeyId keyId{ kPrivateKeyId };
	AliroError err = Aliro::CryptoInstance().DestroyKey(keyId);
	if (err != ALIRO_NO_ERROR) {
		LOG_ERR("Failed to destroy Reader Private Key: %d", err.ToInt());
	}

#ifdef CONFIG_ALIRO_BLE_TP

	keyId = kGroupResolvingKeyId;
	err = Aliro::CryptoInstance().DestroyKey(keyId);
	if (err != ALIRO_NO_ERROR) {
		LOG_ERR("Failed to destroy Group Resolving Key: %d", err.ToInt());
	}

#endif // CONFIG_ALIRO_BLE_TP
}

#endif // CONFIG_CHIP
