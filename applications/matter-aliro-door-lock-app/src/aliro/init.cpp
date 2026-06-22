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

#ifdef CONFIG_DOOR_LOCK_BLE_UWB
#include "aliro/ble_types.h"
#include <aliro_service/aliro_service.h>

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

#include "aliro_state_control.h"
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

constexpr uint8_t GetApplicationFeatures()
{
	uint8_t features = 0;

#ifdef CONFIG_NCS_ALIRO_CREDENTIAL_ISSUER_CA_PUBLIC_KEY
	features |= kFeatureCredentialIssuerCaPublicKeySupported;
#endif // CONFIG_NCS_ALIRO_CREDENTIAL_ISSUER_CA_PUBLIC_KEY

#ifdef CONFIG_DOOR_LOCK_READER_CERTIFICATE
	features |= kFeatureReaderCertificateSupported;
#endif // CONFIG_DOOR_LOCK_READER_CERTIFICATE

	features |= kFeatureMatterSupported;

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

void ClearStorageAliro(bool reinitializeStorage)
{
	auto err = DoorLock::ReaderStorage::ClearAll();
	if (err != ALIRO_NO_ERROR) {
		LOG_ERR("Failed to clear Reader storage: %d", err.ToInt());
	}

#ifdef CONFIG_DOOR_LOCK_CREDENTIAL_ISSUER_CA

	auto keyId = DoorLock::Storage::PsaKeyIds::kCredentialIssuerCAPublicKeyId;
	err = DoorLock::CryptoUtils::DestroyKey(keyId);
	if (err != ALIRO_NO_ERROR) {
		LOG_ERR("Failed to destroy Credential Issuer CA Public Key: %d", err.ToInt());
	}

#endif // CONFIG_DOOR_LOCK_CREDENTIAL_ISSUER_CA

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
