/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "aliro/aliro.h"
#include "aliro/features.h"
#include "aliro/types.h"
#include "aliro/utils.h"
#include "crypto/utils.h"
#include "nfc/nfc_transport_rfal.h"
#include "reader.h"

#ifdef CONFIG_BT
#include "ble_manager.h"
#endif // CONFIG_BT

#ifdef CONFIG_DOOR_LOCK_BLE_UWB
#include "aliro/ble_types.h"
#include "uwb_impl.h"
#endif // CONFIG_DOOR_LOCK_BLE_UWB

#include "access_manager/access_manager.h"
#include "psa_key_ids.h"

#ifdef CONFIG_DOOR_LOCK_CERTIFICATION_TEST_PROVISIONING
#include "certification_provisioning.h"
#endif // CONFIG_DOOR_LOCK_CERTIFICATION_TEST_PROVISIONING

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

#ifndef CONFIG_CHIP
#include "aliro_state_control.h"
#include "lock_sim/lock_sim_instance.h"
#endif // CONFIG_CHIP

#include "aliro_work/aliro_work.h"
#include "storage.h"
#include "storage_keys.h"

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

#ifndef CONFIG_CHIP

AliroError LoadCredentials(KeyValueStorage::KeyIdString pubKeyName, size_t KeyNum, size_t &keyCount)
{
	const auto publicKeyType = (pubKeyName == StorageKeys::kStorageKeyNameAccessCredentialPublicKey) ?
					   AccessManager::PublicKeyType::AccessCredential :
					   AccessManager::PublicKeyType::CredentialIssuer;

	for (size_t keyId = 0; keyId < KeyNum; keyId++) {
		const auto keyName = KeyValueStorage::GetStorageKeyName(pubKeyName, keyId);

		CryptoTypes::PublicKey pubKey{};
		int ec = KeyValueStorage::Instance().Get(keyName.data(), pubKey.data(), pubKey.size());
		VerifyOrReturnStatus(ec == 0 || ec == -ENODATA, ALIRO_ERROR_INTERNAL,
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
	AliroError err{};

#ifdef CONFIG_DOOR_LOCK_CERTIFICATION_TEST_PROVISIONING
	err = DoorLock::CertificationProvisioning::EnsureTestParameters();
	VerifyOrReturnStatus(err == ALIRO_NO_ERROR, err, LOG_ERR("Cannot provision certification test parameters"));
#endif // CONFIG_DOOR_LOCK_CERTIFICATION_TEST_PROVISIONING

	err = LoadAccessCredentials();
	VerifyOrReturnStatus(err == ALIRO_NO_ERROR, err, LOG_ERR("Cannot load Access Credentials"));

	err = LoadIssuerCredentials();
	VerifyOrReturnStatus(err == ALIRO_NO_ERROR, err, LOG_ERR("Cannot load Issuer Credentials"));

	err = DoorLock::Storage::Reader::Init();
	VerifyOrReturnStatus(err == ALIRO_NO_ERROR, err, LOG_ERR("Cannot initialize Reader storage"));

	return ALIRO_NO_ERROR;
}

#endif // CONFIG_CHIP

#ifdef CONFIG_DOOR_LOCK_BLE_UWB

void PrintUwbInfo()
{
#ifdef CONFIG_QM35_UWB_LIB
	using namespace Aliro::Uwb;

	VerifyOrReturn(UltraWideBandImpl::Instance().IsInitialized(), LOG_INF("[UWB] Not initialized yet"));

	const char *fwVersion = UltraWideBandImpl::Instance().GetQm35FirmwareVersion();
	const auto *caps = UltraWideBandImpl::Instance().GetCccCapabilities();

	LOG_INF("[UWB] QM35 FW: %s", fwVersion ? fwVersion : "N/A");

	if (caps) {
		LOG_INF("[UWB] CCC: slots=0x%02x ch=0x%02x hop=0x%02x sync=0x%08x ran_min=%u", caps->mSlotBitmask,
			caps->mChannelBitmask, caps->mHoppingConfigBitmask, caps->mSyncCodeIndexBitmask,
			caps->mMinimumRanMultiplier);
	} else {
		LOG_INF("[UWB] CCC: N/A");
	}
#elif defined(CONFIG_TSRR250_UWB_ALIRO_ZEPHYR)
	LOG_INF("[UWB] TSRR250/SR250 backend selected");
#else
	LOG_INF("[UWB] No hardware backend enabled");
#endif // CONFIG_QM35_UWB_LIB
}

#endif // CONFIG_DOOR_LOCK_BLE_UWB

constexpr uint8_t GetApplicationFeatures()
{
	uint8_t features = 0;

#ifdef CONFIG_DOOR_LOCK_CREDENTIAL_ISSUER_CA
	features |= kFeatureCredentialIssuerCaPublicKeySupported;
#endif // CONFIG_DOOR_LOCK_CREDENTIAL_ISSUER_CA

#ifdef CONFIG_DOOR_LOCK_READER_CERTIFICATE
	features |= kFeatureReaderCertificateSupported;
#endif // CONFIG_DOOR_LOCK_READER_CERTIFICATE

#ifdef CONFIG_CHIP
	features |= kFeatureMatterSupported;
#endif // CONFIG_CHIP

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
	logAppFeature("Matter", (applicationFeatures & kFeatureMatterSupported) != 0);
}

#ifdef CONFIG_DOOR_LOCK_BLE_UWB
AliroError StartAliroAdvertisingImpl(BleManager &bleManager)
{
	Identifier readerIdentifier{};
	AliroError ec = DoorLock::Storage::Reader::GetIdentifier(readerIdentifier);
	VerifyOrReturnStatus(ec == ALIRO_NO_ERROR, ec, LOG_ERR("Failed to get reader identifier"));

	BleTypes::BleAddress address{};
	ec = bleManager.GetAddress(address);
	VerifyOrReturnStatus(ec == ALIRO_NO_ERROR, ec, LOG_ERR("Failed to get BLE address"));

	BleTypes::TxPowerLevel txPower{};
	ec = bleManager.GetTxPowerLevel(txPower);
	VerifyOrReturnStatus(ec == ALIRO_NO_ERROR, ec, LOG_ERR("Failed to get TX power level"));

	BleTypes::AdvertisingServiceData advData{};
	ec = AliroStack::Instance().GenerateAdvertisingData(advData, address, txPower, readerIdentifier);
	VerifyOrReturnStatus(ec == ALIRO_NO_ERROR, ec, LOG_ERR("Failed to get advertising data"));

	ec = bleManager.StartAdvertising(advData);
	VerifyOrReturnStatus(ec == ALIRO_NO_ERROR, ec, LOG_ERR("Failed to start BLE advertising"));

	return ALIRO_NO_ERROR;
}
#endif // CONFIG_DOOR_LOCK_BLE_UWB

} // namespace

#ifdef CONFIG_DOOR_LOCK_BLE_UWB

AliroError StartAliroAdvertising()
{
	return StartAliroAdvertisingImpl(BleManager::Instance());
}

#endif // CONFIG_DOOR_LOCK_BLE_UWB

int AliroInit()
{
	AliroError ec{};
	LOG_INF("Starting nRF Door Lock and Access Control Application");

	std::ignore = AliroWorkInit();

#ifdef CONFIG_BT
	// Initialize BLE manager first (app-controlled BLE stack)
	ec = BleManager::Instance().Init();
	VerifyOrReturnValue(ec == ALIRO_NO_ERROR, EXIT_FAILURE, LOG_ERR("BLE manager initialization failed"));
#endif // CONFIG_BT

	ec = AliroStack::Instance().Init();
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

#ifndef CONFIG_CHIP
	LockSimInstance().Init([]([[maybe_unused]] OperationSource source, [[maybe_unused]] ReaderStateByte state) {
#ifdef CONFIG_DOOR_LOCK_BLE_UWB
		Aliro::AliroStack::Instance().SendReaderStatusChangedMessage(source, state);
		auto &bleManager = BleManager::Instance();
		AliroError ec{};
		if (state == ReaderStateByte::Unsecured) {
			ec = bleManager.StopAdvertising();
			VerifyOrReturn(ec == ALIRO_NO_ERROR, LOG_ERR("Failed to stop Aliro advertising"));
		} else if (state == ReaderStateByte::EnteringSecured) {
			ec = StartAliroAdvertisingImpl(bleManager);
			VerifyOrReturn(ec == ALIRO_NO_ERROR, LOG_ERR("Failed to start Aliro advertising"));
		}
#endif // CONFIG_DOOR_LOCK_BLE_UWB
	});

	AccessManagerInstance().SetApplicationCallbacks(
		{ .mUnlockIndicatorClb =
			  [](OperationSource source) {
				  const auto isNfcSession = source == OperationSource::ThisUserDeviceInNfc;
				  LOG_INF("Door unlocked via %s session", isNfcSession ? "NFC" : "BLE/UWB");
				  if (!LockSimInstance().Unlock(source)) {
#ifdef CONFIG_DOOR_LOCK_BLE_UWB
					  // The lock is already unlocked, so we can send the Unsecured state
					  Aliro::AliroStack::Instance().SendReaderStatusChangedMessage(
						  source, ReaderStateByte::Unsecured);
#endif // CONFIG_DOOR_LOCK_BLE_UWB
				  }
			  },
		  .mLockIndicatorClb =
			  [](OperationSource source) {
				  const auto isNfcSession = source == OperationSource::ThisUserDeviceInNfc;
				  LOG_INF("Door locked via %s session", isNfcSession ? "NFC" : "BLE/UWB");
				  LockSimInstance().Lock(source);
			  },
		  .mAccessIndicatorClb =
			  [](bool isAccessGranted, bool isNfcSession) {
				  LOG_INF("Access %s via %s session", isAccessGranted ? "granted" : "denied",
					  isNfcSession ? "NFC" : "BLE/UWB");
			  } });

	ec = StorageInit();
	VerifyOrReturnValue(ec == ALIRO_NO_ERROR, EXIT_FAILURE, LOG_ERR("Storage initialization failed"));

#endif // CONFIG_CHIP

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
#ifdef CONFIG_DOOR_LOCK_BLE_UWB
	PrintUwbInfo();
#endif // CONFIG_DOOR_LOCK_BLE_UWB

	LOG_INF("Aliro stack initialized");

#ifndef CONFIG_CHIP
	ec = DoorLock::AliroStateControl::UpdateAliroState();
	VerifyOrReturnValue(ec == ALIRO_NO_ERROR, EXIT_FAILURE,
			    LOG_ERR("Failed to update Aliro state: %d", ec.ToInt()));
#endif // CONFIG_CHIP

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
	ec = DoorLock::Crypto::IsKeyAvailable(DoorLock::Storage::PsaKeyIds::kGroupResolvingKeyId);
	if (ec != ALIRO_NO_ERROR) {
		LOG_DBG("Group Resolving Key is not provisioned, all-zero key will be used");
		CryptoTypes::GroupResolvingKey groupResolvingKey{};
		CryptoTypes::KeyId groupResolvingKeyId = DoorLock::Storage::PsaKeyIds::kGroupResolvingKeyId;
		ec = DoorLock::Crypto::ImportGroupResolvingKey(groupResolvingKey, true, groupResolvingKeyId);
		VerifyOrReturnValue(ec == ALIRO_NO_ERROR, EXIT_FAILURE, LOG_ERR("Cannot import group resolving key"));
	}

	auto &bleManager = BleManager::Instance();
	ec = StartAliroAdvertisingImpl(bleManager);
	VerifyOrReturnValue(ec == ALIRO_NO_ERROR, EXIT_FAILURE, LOG_ERR("Failed to start Aliro advertising"));
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
	ec = BleManager::Instance().StopAdvertising();
	if (ec != ALIRO_NO_ERROR) {
		LOG_ERR("Failed to stop BLE advertising: %d", ec.ToInt());
	}
	ec = BleManager::Instance().DisconnectAll();
	if (ec != ALIRO_NO_ERROR) {
		LOG_ERR("Failed to disconnect all BLE connections");
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

#ifdef CONFIG_CHIP

void ClearStorageAliro(bool reinitializeStorage)
{
	int ec = KeyValueStorage::Instance().Clear(StorageKeys::kStorageKeyNameIdentifier);
	if (!ec) {
		LOG_ERR("Failed to clear %s/%s storage key: %d", StorageKeys::kDoorLockBaseKey,
			StorageKeys::kStorageKeyNameIdentifier, ec);
	}

	CryptoTypes::KeyId keyId{ DoorLock::Storage::PsaKeyIds::kPrivateKeyId };
	AliroError err = DoorLock::Crypto::DestroyKey(keyId);
	if (err != ALIRO_NO_ERROR) {
		LOG_ERR("Failed to destroy Reader Private Key: %d", err.ToInt());
	}

#ifdef CONFIG_DOOR_LOCK_CREDENTIAL_ISSUER_CA

	keyId = DoorLock::Storage::PsaKeyIds::kCredentialIssuerCAPublicKeyId;
	err = DoorLock::Crypto::DestroyKey(keyId);
	if (err != ALIRO_NO_ERROR) {
		LOG_ERR("Failed to destroy Credential Issuer CA Public Key: %d", err.ToInt());
	}

#endif // CONFIG_DOOR_LOCK_CREDENTIAL_ISSUER_CA

#ifdef CONFIG_DOOR_LOCK_BLE_UWB

	keyId = DoorLock::Storage::PsaKeyIds::kGroupResolvingKeyId;
	err = DoorLock::Crypto::DestroyKey(keyId);
	if (err != ALIRO_NO_ERROR) {
		LOG_ERR("Failed to destroy Group Resolving Key: %d", err.ToInt());
	}

#endif // CONFIG_DOOR_LOCK_BLE_UWB

#ifdef CONFIG_DOOR_LOCK_EXPEDITED_FAST_PHASE

	sKpersistentManagerImpl.RemoveAllKpersistent();

#endif // CONFIG_DOOR_LOCK_EXPEDITED_FAST_PHASE

#ifdef CONFIG_DOOR_LOCK_EXTERNAL_NVS
	DoorLock::ExternalNvs::Clear();
	if (reinitializeStorage) {
		DoorLock::ExternalNvs::Init(FIXED_PARTITION_ID(external_nvs));
	}
#else // CONFIG_DOOR_LOCK_EXTERNAL_NVS
	ARG_UNUSED(reinitializeStorage);
#endif // CONFIG_DOOR_LOCK_EXTERNAL_NVS
}

#endif // CONFIG_CHIP
