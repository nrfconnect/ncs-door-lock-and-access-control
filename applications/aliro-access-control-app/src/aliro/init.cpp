/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "aliro/aliro.h"
#include "aliro/features.h"
#include "aliro/types.h"
#include "aliro/utils.h"
#include "nfc/nfc_transport_rfal.h"

#include <reader_storage/reader.h>

#ifdef CONFIG_DOOR_LOCK_NUS_SERVICE
#include <nus_service/nus_service.h>
#endif // CONFIG_DOOR_LOCK_NUS_SERVICE

#ifdef CONFIG_DOOR_LOCK_BLE_UWB
#include "aliro/ble_types.h"
#include <aliro_service/aliro_service.h>

#include "last_operation.h"
#include "uwb_impl.h"
#endif // CONFIG_DOOR_LOCK_BLE_UWB

#include "access_manager/access_manager.h"
#include "psa_key_ids.h"
#include "psa_ps_ids.h"

#ifdef CONFIG_DOOR_LOCK_EXPEDITED_FAST_PHASE
#include "kpersistent_manager/kpersistent_manager_impl.h"
#endif // CONFIG_DOOR_LOCK_EXPEDITED_FAST_PHASE

#if defined(CONFIG_DOOR_LOCK_STEP_UP_PHASE) && CONFIG_DOOR_LOCK_STORAGE_MAX_STORED_ACCESS_DOCUMENTS > 0
#include "access_document.h"
#endif // CONFIG_DOOR_LOCK_STEP_UP_PHASE AND CONFIG_DOOR_LOCK_STORAGE_MAX_STORED_ACCESS_DOCUMENTS > 0

#ifdef CONFIG_DOOR_LOCK_EXTERNAL_NVS
#include <external_nvs/external_nvs.h>
#include <zephyr/storage/flash_map.h>
#endif // CONFIG_DOOR_LOCK_EXTERNAL_NVS

#ifdef CONFIG_DOOR_LOCK_CLI
#include "shell.h"
#endif // CONFIG_DOOR_LOCK_CLI

#ifdef CONFIG_DOOR_LOCK_CERTIFICATION_TEST_PROVISIONING
#include "certification_provisioning.h"
#endif // CONFIG_DOOR_LOCK_CERTIFICATION_TEST_PROVISIONING

#include "aliro_state_control.h"
#include "lock_sim_instance.h"
#include "storage.h"
#include "storage_keys.h"

#include <crypto_utils/crypto_utils.h>

#include <zephyr/logging/log.h>
#include <zephyr/shell/shell.h>

#include <cstdio>
#include <stdlib.h>
#include <tuple>

LOG_MODULE_REGISTER(aliro, CONFIG_DOOR_LOCK_APP_LOG_LEVEL);

using namespace Aliro;

#ifdef CONFIG_DOOR_LOCK_EXPEDITED_FAST_PHASE

KpersistentManagerImpl sKpersistentManagerImpl;

#endif // CONFIG_DOOR_LOCK_EXPEDITED_FAST_PHASE

namespace {

bool sAliroRunning{ false };

AliroError LoadCredentials(KeyValueStorage::KeyIdString pubKeyName, size_t KeyNum, size_t &keyCount)
{
	const auto publicKeyType = (pubKeyName == StorageKeys::kStorageKeyNameAccessCredentialPublicKey) ?
					   AccessManager::PublicKeyType::AccessCredential :
					   AccessManager::PublicKeyType::CredentialIssuer;

	for (size_t keyId = 0; keyId < KeyNum; keyId++) {
		const auto keyName = KeyValueStorage::GetStorageKeyName(pubKeyName, keyId);

		CryptoTypes::PublicKey pubKey{};
		int ec = KeyValueStorage::Instance().Get(keyName.data(), pubKey.data(), pubKey.size());
		VerifyOrReturnStatus(ec == 0 || ec == -ENOENT, ALIRO_ERROR_INTERNAL,
				     LOG_ERR("Cannot get %s/%d public key, error code: %d", pubKeyName, keyId, ec));
		if (ec == 0) {
			AliroError err = AccessManagerInstance().AddPublicKey(pubKey, publicKeyType, keyId);
			VerifyOrReturnStatus(err == ALIRO_NO_ERROR, err, LOG_ERR("Cannot set public key."));
			++keyCount;
		}
	}

	return ALIRO_NO_ERROR;
}

AliroError LoadAccessCredentials()
{
	size_t keyCount{};
	ReturnErrorOnFailure(LoadCredentials(StorageKeys::kStorageKeyNameAccessCredentialPublicKey,
					     CONFIG_DOOR_LOCK_ACCESS_MANAGER_ACCESS_CREDENTIAL_MAX_STORED_KEYS,
					     keyCount));

	if (keyCount == 0) {
		LOG_INF("No Access Credential public keys available");
	}

	return ALIRO_NO_ERROR;
}

AliroError LoadIssuerCredentials()
{
#if CONFIG_DOOR_LOCK_ACCESS_MANAGER_CREDENTIAL_ISSUER_MAX_STORED_KEYS > 0

	size_t keyCount{};
	ReturnErrorOnFailure(LoadCredentials(StorageKeys::kStorageKeyNameCredentialIssuerPublicKey,
					     CONFIG_DOOR_LOCK_ACCESS_MANAGER_CREDENTIAL_ISSUER_MAX_STORED_KEYS,
					     keyCount));

	if (keyCount == 0) {
		LOG_INF("No Credential Issuer public keys available");
	}

#endif // CONFIG_DOOR_LOCK_ACCESS_MANAGER_CREDENTIAL_ISSUER_MAX_STORED_KEYS > 0

	return ALIRO_NO_ERROR;
}

AliroError StorageInit()
{
	AliroError err = LoadAccessCredentials();
	VerifyOrReturnStatus(err == ALIRO_NO_ERROR, err, LOG_ERR("Cannot load Access Credentials"));

	err = LoadIssuerCredentials();
	VerifyOrReturnStatus(err == ALIRO_NO_ERROR, err, LOG_ERR("Cannot load Issuer Credentials"));

	return ALIRO_NO_ERROR;
}

constexpr uint8_t GetApplicationFeatures()
{
	uint8_t features = 0;

#ifdef CONFIG_DOOR_LOCK_CREDENTIAL_ISSUER_CA
	features |= kFeatureCredentialIssuerCaPublicKeySupported;
#endif // CONFIG_DOOR_LOCK_CREDENTIAL_ISSUER_CA

#ifdef CONFIG_DOOR_LOCK_READER_CERTIFICATE
	features |= kFeatureReaderCertificateSupported;
#endif // CONFIG_DOOR_LOCK_READER_CERTIFICATE

	return features;
}

void PrintAliroFeatures(uint8_t stackFeatures, uint8_t applicationFeatures)
{
	const auto logStackFeature = [](const char *name, bool enabled) {
		LOG_INF("[Aliro] %s: %u", name, enabled ? 1U : 0U);
	};

	const auto logAppFeature = [](const char *name, bool enabled) {
		LOG_INF("[Doorlock] %s: %u", name, enabled ? 1U : 0U);
	};

	LOG_INF("[Aliro] Stack features mask: 0x%02x", static_cast<unsigned int>(stackFeatures));
	logStackFeature("ExpFast", (stackFeatures & kFeatureExpeditedFastPhaseSupported) != 0);
	logStackFeature("StepUp", (stackFeatures & kFeatureStepUpPhaseSupported) != 0);
	logStackFeature("UWB", (stackFeatures & kFeatureBleUwbSupported) != 0);

	LOG_INF("[Doorlock] Application features mask: 0x%02x", static_cast<unsigned int>(applicationFeatures));
	logAppFeature("CredCA", (applicationFeatures & kFeatureCredentialIssuerCaPublicKeySupported) != 0);
	logAppFeature("ReaderCert", (applicationFeatures & kFeatureReaderCertificateSupported) != 0);
}

#ifdef CONFIG_DOOR_LOCK_NUS_SERVICE

template <size_t N> constexpr std::pair<const char *, size_t> GetNusServiceMessagePair(const char (&str)[N])
{
	return std::make_pair(str, N);
}

constexpr std::pair<const char *, size_t> GetNusServiceMessage(ReaderStateByte state)
{
	switch (state) {
	case ReaderStateByte::Secured:
		return GetNusServiceMessagePair("locked");
	case ReaderStateByte::Unsecured:
		return GetNusServiceMessagePair("unlocked");
	case ReaderStateByte::Blocked:
		return GetNusServiceMessagePair("blocked");
	case ReaderStateByte::EnteringSecured:
		return GetNusServiceMessagePair("locking");
	case ReaderStateByte::EnteringUnsecured:
		return GetNusServiceMessagePair("unlocking");
	default:
		return GetNusServiceMessagePair("unknown");
	}
}

#endif // CONFIG_DOOR_LOCK_NUS_SERVICE

#ifdef CONFIG_DOOR_LOCK_BLE_UWB

void SendReaderStatusChangedMessage(ReaderStateByte state)
{
	OperationSource source{ OperationSource::Auto };
	const CryptoTypes::PublicKey *accessCredentialPublicKey{ nullptr };

	const auto lastOperation = GetLastOperation();
	source = lastOperation.source;
	if (lastOperation.accessCredentialPublicKey.has_value()) {
		accessCredentialPublicKey = &lastOperation.accessCredentialPublicKey.value();
	}

	AliroStack::Instance().SendReaderStatusChangedMessage(source, state, accessCredentialPublicKey);
}

#endif // CONFIG_DOOR_LOCK_BLE_UWB

} // namespace

int AliroInit()
{
	LOG_INF("Starting nRF Door Lock and Access Control Application");

#ifdef CONFIG_DOOR_LOCK_BLE_UWB
	const int aliroServiceRc = DoorLock::AliroService::Init();
	VerifyOrReturnValue(aliroServiceRc == 0, EXIT_FAILURE,
			    LOG_ERR("Aliro service initialization failed: %d", aliroServiceRc));
#endif // CONFIG_DOOR_LOCK_BLE_UWB

	AliroError ec = AliroStack::Instance().Init();
	VerifyOrReturnValue(ec == ALIRO_NO_ERROR, EXIT_FAILURE, LOG_ERR("Aliro stack initialization failed"));

	ec = Aliro::NfcTransportRfal::Instance().Init();
	if (ec != ALIRO_NO_ERROR) {
		LOG_ERR("NFC transport initialization failed");
	}

	KpersistentManager *kpersistentManager{ nullptr };

#ifdef CONFIG_DOOR_LOCK_EXPEDITED_FAST_PHASE
	sKpersistentManagerImpl.Init();
	AccessManagerInstanceImpl().SetKpersistentManager(&sKpersistentManagerImpl);
	kpersistentManager = &sKpersistentManagerImpl;
#endif // CONFIG_DOOR_LOCK_EXPEDITED_FAST_PHASE

	LockSimInstance().Init([](ReaderStateByte state) {
#ifdef CONFIG_DOOR_LOCK_BLE_UWB
		SendReaderStatusChangedMessage(state);
#ifdef CONFIG_DOOR_LOCK_NUS_SERVICE
		const auto [message, messageLength] = GetNusServiceMessage(state);
		DoorLock::NUSService::Send(message, messageLength);
#endif // CONFIG_DOOR_LOCK_NUS_SERVICE
		using namespace DoorLock::AliroService;
		if (state == ReaderStateByte::Unsecured) {
			const auto error = BlockAdvertising(AdvertisingBlockReason::DoorUnlocked);
			VerifyOrReturn(error == ALIRO_NO_ERROR, LOG_ERR("Failed to stop Aliro advertising"));
		} else if (state == ReaderStateByte::EnteringSecured) {
			const auto error = UnblockAdvertising(AdvertisingBlockReason::DoorUnlocked);
			VerifyOrReturn(error == ALIRO_NO_ERROR, LOG_ERR("Failed to start Aliro advertising"));
		}
#endif // CONFIG_DOOR_LOCK_BLE_UWB
	});

	AccessManagerInstance().SetApplicationCallbacks(
		{ .mUnlockIndicatorClb =
			  []([[maybe_unused]] bool isNfcSession,
			     [[maybe_unused]] const CryptoTypes::PublicKey &accessCredentialPublicKey) {
				  LOG_DBG("Door unlocked via %s session", isNfcSession ? "NFC" : "BLE/UWB");
#ifdef CONFIG_DOOR_LOCK_BLE_UWB
				  SetLastOperation(isNfcSession, accessCredentialPublicKey);
#endif // CONFIG_DOOR_LOCK_BLE_UWB
				  if (!LockSimInstance().Unlock()) {
#ifdef CONFIG_DOOR_LOCK_BLE_UWB
					  // The lock is already unlocked, so we can send the Unsecured state
					  SendReaderStatusChangedMessage(ReaderStateByte::Unsecured);
#endif // CONFIG_DOOR_LOCK_BLE_UWB
				  }
			  },
		  .mLockIndicatorClb =
			  []([[maybe_unused]] bool isNfcSession,
			     [[maybe_unused]] const CryptoTypes::PublicKey &accessCredentialPublicKey) {
				  LOG_DBG("Door locked via %s session", isNfcSession ? "NFC" : "BLE/UWB");
#ifdef CONFIG_DOOR_LOCK_BLE_UWB
				  SetLastOperation(isNfcSession, accessCredentialPublicKey);
#endif // CONFIG_DOOR_LOCK_BLE_UWB
				  LockSimInstance().Lock();
			  },
		  .mAccessIndicatorClb =
			  []([[maybe_unused]] bool isAccessGranted, [[maybe_unused]] bool isNfcSession) {
				  LOG_INF("Access %s via %s session", isAccessGranted ? "granted" : "denied",
					  isNfcSession ? "NFC" : "BLE/UWB");
			  } });

#ifdef CONFIG_DOOR_LOCK_CERTIFICATION_TEST_PROVISIONING
	ec = DoorLock::CertificationProvisioning::EnsureTestParameters();
	VerifyOrReturnValue(ec == ALIRO_NO_ERROR, EXIT_FAILURE,
			    LOG_ERR("Cannot provision certification test parameters"));
#endif // CONFIG_DOOR_LOCK_CERTIFICATION_TEST_PROVISIONING

	ec = StorageInit();
	VerifyOrReturnValue(ec == ALIRO_NO_ERROR, EXIT_FAILURE, LOG_ERR("Storage initialization failed"));

	ec = DoorLock::ReaderStorage::Init(
#ifdef CONFIG_DOOR_LOCK_ALIRO_READER_STORAGE_SHELL
		[]() { DoorLock::AliroStateControl::UpdateAliroState(); }
#endif // CONFIG_DOOR_LOCK_ALIRO_READER_STORAGE_SHELL
	);
	VerifyOrReturnValue(ec == ALIRO_NO_ERROR, EXIT_FAILURE, LOG_ERR("Cannot initialize Reader storage"));

#ifdef CONFIG_DOOR_LOCK_EXTERNAL_NVS
	auto initRc = DoorLock::ExternalNvs::Init(FIXED_PARTITION_ID(external_nvs));
	VerifyOrReturnValue(initRc == 0, EXIT_FAILURE, LOG_ERR("External NVS init failed: %d", initRc));
#endif // CONFIG_DOOR_LOCK_EXTERNAL_NVS

#if defined(CONFIG_DOOR_LOCK_STEP_UP_PHASE) && CONFIG_DOOR_LOCK_STORAGE_MAX_STORED_ACCESS_DOCUMENTS > 0
	ec = LoadAccessDocuments();
	VerifyOrReturnValue(ec == ALIRO_NO_ERROR, EXIT_FAILURE, LOG_ERR("Cannot load Access Documents"));
#endif // CONFIG_DOOR_LOCK_STEP_UP_PHASE && CONFIG_DOOR_LOCK_STORAGE_MAX_STORED_ACCESS_DOCUMENTS > 0

#ifdef CONFIG_DOOR_LOCK_CLI
	InitShellCommands(kpersistentManager);
#endif // CONFIG_DOOR_LOCK_CLI

	PrintAliroFeatures(AliroStack::Instance().GetFeatures(), GetApplicationFeatures());

	LOG_INF("Aliro stack initialized");

	ec = DoorLock::AliroStateControl::UpdateAliroState();
	VerifyOrReturnValue(ec == ALIRO_NO_ERROR, EXIT_FAILURE,
			    LOG_ERR("Failed to update Aliro state: %d", ec.ToInt()));

	return EXIT_SUCCESS;
}

int AliroStart()
{
	AliroError ec = NfcTransportRfal::Instance().Start();
	if (ec != ALIRO_NO_ERROR) {
		LOG_ERR("NFC transport start failed");
		return EXIT_FAILURE;
	}

#ifdef CONFIG_DOOR_LOCK_BLE_UWB

	// Ensure Group Resolving Key exists before starting BLE advertising
	if (!DoorLock::ReaderStorage::IsGroupResolvingKeySet()) {
		LOG_DBG("Group Resolving Key is not provisioned, all-zero key will be used");
		CryptoTypes::GroupResolvingKey groupResolvingKey{};
		ec = DoorLock::ReaderStorage::SetGroupResolvingKey(groupResolvingKey);
		VerifyOrReturnValue(ec == ALIRO_NO_ERROR, EXIT_FAILURE, LOG_ERR("Cannot set Group Resolving Key"));
	}

	const int aliroServiceStartRc = DoorLock::AliroService::Start();
	VerifyOrReturnValue(aliroServiceStartRc == 0, EXIT_FAILURE,
			    LOG_ERR("Failed to start Aliro service: %d", aliroServiceStartRc));

#endif // CONFIG_DOOR_LOCK_BLE_UWB

	sAliroRunning = true;
	return EXIT_SUCCESS;
}

int AliroStop()
{
	int rc = EXIT_SUCCESS;

	AliroError ec = NfcTransportRfal::Instance().Stop();
	if (ec != ALIRO_NO_ERROR) {
		LOG_ERR("NFC transport stop failed");
	}

#ifdef CONFIG_DOOR_LOCK_BLE_UWB

	const int aliroServiceStopRc = DoorLock::AliroService::Stop();
	if (aliroServiceStopRc != 0) {
		LOG_ERR("Failed to stop Aliro service: %d", aliroServiceStopRc);
		rc = EXIT_FAILURE;
	}

#endif // CONFIG_DOOR_LOCK_BLE_UWB

	sAliroRunning = false;
	return rc;
}

bool IsAliroRunning()
{
	return sAliroRunning;
}
