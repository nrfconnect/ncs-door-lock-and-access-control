/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "shell.h"

#include <aliro/aliro.h>
#include <aliro/memory.h>
#include <aliro/utils.h>

#ifdef CONFIG_DOOR_LOCK_BLE_UWB
#include "ble/ble_manager.h"
#endif // CONFIG_DOOR_LOCK_BLE_UWB

#ifdef CONFIG_DOOR_LOCK_READER_CERTIFICATE
#include "reader_certificate_cache.h"
#endif // CONFIG_DOOR_LOCK_READER_CERTIFICATE

#include "aliro/crypto_key_ids.h"

#include <zephyr/shell/shell.h>

#ifdef CONFIG_DOOR_LOCK_DFU_BLE_SMP_STANDALONE
#include "dfu_smp_shell.h"
#endif // CONFIG_DOOR_LOCK_DFU_BLE_SMP_STANDALONE

#ifndef CONFIG_CHIP

#include "access_manager/access_manager.h"
#include "crypto/crypto.h"
#include "storage.h"
#include "storage_keys.h"

#ifdef CONFIG_DOOR_LOCK_STEP_UP_PHASE
#include "validity_iterations.h"
#endif // CONFIG_DOOR_LOCK_STEP_UP_PHASE

#ifdef CONFIG_SETTINGS_NVS
#include <zephyr/fs/nvs.h>
#else // CONFIG_SETTINGS_ZMS
#include <zephyr/fs/zms.h>
#endif // CONFIG_SETTINGS_NVS || CONFIG_SETTINGS_ZMS

#include <zephyr/settings/settings.h>
#include <zephyr/sys/reboot.h>
#include <zephyr/sys/util.h>

#endif // CONFIG_CHIP

#include <cstdlib>

namespace {
using namespace Aliro;

constexpr size_t kPublicKeyStringLength{ 2 * CryptoTypes::kEccP256PublicKeyLength };

// Flag to check if shell is already initialized.
bool isInitialized{ false };

#ifdef CONFIG_DOOR_LOCK_EXPEDITED_FAST_PHASE
KpersistentManager *sKpersistentManager{ nullptr };
#endif // CONFIG_DOOR_LOCK_EXPEDITED_FAST_PHASE

template <size_t N> constexpr size_t CStrLen(const char (&)[N])
{
	return N;
}

template <size_t N> bool CmdMatch(const char *cmd, const char (&cmdStr)[N])
{
	return strncmp(cmd, cmdStr, CStrLen(cmdStr)) == 0;
}

#ifndef CONFIG_CHIP

using PublicKeyType = AccessManager::PublicKeyType;

constexpr char kCmdReaderIdentifier[] = "identifier";
constexpr char kCmdReaderGroupIdentifier[] = "group_id";
constexpr char kCmdReaderGroupSubIdentifier[] = "group_sub_id";
constexpr size_t kAcMaxKeys{ CONFIG_DOOR_LOCK_ACCESS_MANAGER_ACCESS_CREDENTIAL_MAX_STORED_KEYS };
[[maybe_unused]] constexpr size_t kCiMaxKeys{ CONFIG_DOOR_LOCK_ACCESS_MANAGER_CREDENTIAL_ISSUER_MAX_STORED_KEYS };

K_WORK_DELAYABLE_DEFINE(sRebootWork, [](k_work *) { sys_reboot(SYS_REBOOT_WARM); });

constexpr PublicKeyType GetPublicKeyTypeFromStorageKey(KeyValueStorage::KeyIdString keyName)
{
	return keyName == StorageKeys::kStorageKeyNameAccessCredentialPublicKey ? PublicKeyType::AccessCredential :
										  PublicKeyType::CredentialIssuer;
}

int ShellCmdHandleIdentifiers(const struct shell *shell, size_t argc, char **argv)
{
	VerifyOrReturnStatus(IN_RANGE(argc, 1, 2), -EINVAL, shell_warn(shell, "Invalid number of arguments!\n"));
	VerifyOrReturnValue(isInitialized, -EIO, shell_warn(shell, "Not initialized yet\n"));

	Identifier identifier{};
	std::array<char, kReaderIdentifierLength * 2 + 1> identifierStr{};

	int status = KeyValueStorage::Instance().Get(StorageKeys::kStorageKeyNameIdentifier, identifier.data(),
						     identifier.size());

	size_t offset{ 0 };
	size_t length{ 0 };

	if (CmdMatch(argv[0], kCmdReaderIdentifier)) {
		offset = 0;
		length = identifier.size();
	} else if (CmdMatch(argv[0], kCmdReaderGroupIdentifier)) {
		offset = 0;
		length = kReaderGroupIdentifierLength;
	} else if (CmdMatch(argv[0], kCmdReaderGroupSubIdentifier)) {
		offset = kReaderGroupIdentifierLength;
		length = kReaderGroupSubIdentifierLength;
	} else {
		return -EINVAL;
	}

	if (argc == 1) {
		VerifyOrReturnStatus(status == 0, status,
				     shell_warn(shell, "Cannot get %s, error: %d\n",
						StorageKeys::kStorageKeyNameIdentifier, status));

		bin2hex(identifier.data() + offset, length, identifierStr.data(), identifierStr.size());
		shell_print(shell, "%s", identifierStr.data());

		return 0;
	}

	size_t argLength = strlen(argv[1]);

	VerifyOrReturnStatus(argLength == length * 2, -EINVAL, shell_warn(shell, "Invalid %s length!\n", argv[0]));

	hex2bin(argv[1], argLength, identifier.data() + offset, length);

	VerifyOrReturnStatus(!KeyValueStorage::Instance().Save(StorageKeys::kStorageKeyNameIdentifier,
							       identifier.data(), identifier.size()),
			     -EINVAL, shell_warn(shell, "Cannot update %s\n", StorageKeys::kStorageKeyNameIdentifier));

	AliroError aliroError = AliroStack::Instance().SetReaderIdentifier(identifier);
	VerifyOrReturnStatus(aliroError == ALIRO_NO_ERROR, -EINVAL,
			     shell_warn(shell, "Failed to set reader identifier\n"));

#ifdef CONFIG_DOOR_LOCK_BLE_UWB
	// Update BLE advertising data with new reader identifier
	auto &bleManager = BleManager::Instance();

	BleTypes::BleAddress address{};
	BleTypes::TxPowerLevel txPower{};
	BleTypes::AdvertisingServiceData advData{};

	aliroError = bleManager.GetAddress(address);
	VerifyOrReturnStatus(aliroError == ALIRO_NO_ERROR, -EINVAL, shell_warn(shell, "Failed to get BLE address\n"));

	aliroError = bleManager.GetTxPowerLevel(txPower);
	VerifyOrReturnStatus(aliroError == ALIRO_NO_ERROR, -EINVAL,
			     shell_warn(shell, "Failed to get TX power level\n"));

	aliroError = AliroStack::Instance().GenerateAdvertisingData(advData, address, txPower, identifier);
	VerifyOrReturnStatus(aliroError == ALIRO_NO_ERROR, -EINVAL,
			     shell_warn(shell, "Failed to get advertising data\n"));

	aliroError = bleManager.UpdateAdvertisingData(advData);
	VerifyOrReturnStatus(aliroError == ALIRO_NO_ERROR, -EINVAL,
			     shell_warn(shell, "Failed to update advertising data\n"));
#endif // CONFIG_DOOR_LOCK_BLE_UWB

	return 0;
}

int GetPublicKeyFromStorage(KeyValueStorage::KeyIdString keyNameInStorage, size_t keyId, CryptoTypes::PublicKey &pubKey)
{
	const auto keyName = KeyValueStorage::GetStorageKeyName(keyNameInStorage, keyId);

	return KeyValueStorage::Instance().Get(keyName.data(), pubKey.data(), pubKey.size());
}

int FindPublicKeyInStorage(KeyValueStorage::KeyIdString keyNameInStorage, const CryptoTypes::PublicKey &pubKey,
			   size_t numSlots)
{
	for (size_t keyId = 0; keyId < numSlots; keyId++) {
		CryptoTypes::PublicKey publicKey{};
		if (GetPublicKeyFromStorage(keyNameInStorage, keyId, publicKey) == 0 && publicKey == pubKey) {
			return keyId;
		}
	}

	return -ENOENT;
}

AliroError RemovePublicKeyFromAccessManager(KeyValueStorage::KeyIdString keyIdStr, size_t keyId)
{
	CryptoTypes::PublicKey publicKey{};
	auto publicKeyType = GetPublicKeyTypeFromStorageKey(keyIdStr);

	if (GetPublicKeyFromStorage(keyIdStr, keyId, publicKey) == 0) {
		ReturnErrorOnFailure(AccessManagerInstance().RemovePublicKey(publicKeyType, keyId));
	}

	return ALIRO_NO_ERROR;
}

int RemoveKey(const struct shell *shell, KeyValueStorage::KeyIdString keyIdStr, size_t keyId)
{
	VerifyOrReturnStatus(RemovePublicKeyFromAccessManager(keyIdStr, keyId) == ALIRO_NO_ERROR, -EINVAL,
			     shell_warn(shell, "Cannot remove Access Credential public key from Access Manager\n"));

	const auto keyName = KeyValueStorage::GetStorageKeyName(keyIdStr, keyId);
	VerifyOrReturnStatus(!KeyValueStorage::Instance().Clear(keyName.data()), -EIO,
			     shell_warn(shell, "Cannot clear key-value in persistent storage!\n"));

	return 0;
}

int ParseKeyId(const struct shell *shell, KeyValueStorage::KeyIdString keyIdStr, size_t &keyId, size_t numSlots)
{
	char *endPtr{ nullptr };
	errno = 0;
	unsigned long val = strtoul(keyIdStr, &endPtr, 10);
	VerifyOrReturnStatus(errno == 0 && endPtr != keyIdStr, -EINVAL, shell_warn(shell, "Invalid key ID!\n"));

	VerifyOrReturnStatus(numSlots > 0, -EINVAL,
			     shell_warn(shell, "Access Credential public key slots not supported!\n"));

	VerifyOrReturnStatus(IN_RANGE(val, 0, numSlots - 1), -EINVAL,
			     shell_warn(shell, "Key ID out of range, must be between 0 and %u!\n", numSlots - 1));

	keyId = val;
	return 0;
}

int CmdHandleAndListKey(const struct shell *shell, KeyValueStorage::KeyIdString keyIdStr, size_t numSlots)
{
	VerifyOrReturnValue(isInitialized, -EIO, shell_warn(shell, "Not initialized yet\n"));

	std::array<char, kPublicKeyStringLength + 1> hexString{};
	CryptoTypes::PublicKey publicKey{};

	for (size_t keyId = 0; keyId < numSlots; keyId++) {
		if (GetPublicKeyFromStorage(keyIdStr, keyId, publicKey) == 0) {
			bin2hex(publicKey.data(), publicKey.size(), hexString.data(), hexString.size());
		} else {
			snprintf(hexString.data(), hexString.size(), "(null)");
		}

		shell_print(shell, "[%u]: %s", keyId, hexString.data());
	}

	return 0;
}

int CmdHandleAndClearKey(const struct shell *shell, size_t argc, char **argv, KeyValueStorage::KeyIdString keyIdStr,
			 size_t numSlots, size_t &keyId)
{
	VerifyOrReturnStatus(argc == 2, -EINVAL, shell_warn(shell, "Invalid number of arguments!\n"));
	VerifyOrReturnValue(isInitialized, -EIO, shell_warn(shell, "Not initialized yet\n"));

	constexpr char kAll[] = "all";
	if (strncmp(argv[1], kAll, CStrLen(kAll)) == 0) {
		shell_warn(shell, "Clearing all %s public keys!\n", keyIdStr);
		for (size_t keyId = 0; keyId < numSlots; keyId++) {
			int status = RemoveKey(shell, keyIdStr, keyId);
			VerifyOrReturnStatus(status == 0, status);
		}
		return 0;
	}

	int status = ParseKeyId(shell, argv[1], keyId, numSlots);
	VerifyOrReturnStatus(status == 0, status);

	status = RemoveKey(shell, keyIdStr, keyId);
	VerifyOrReturnStatus(status == 0, status);
	return 0;
}

int CmdHandleAndSetKey(const struct shell *shell, size_t argc, char **argv, KeyValueStorage::KeyIdString keyIdStr,
		       size_t numSlots, size_t &keyId)
{
	VerifyOrReturnStatus(argc == 3, -EINVAL, shell_warn(shell, "Invalid number of arguments!\n"));
	VerifyOrReturnValue(isInitialized, -EIO, shell_warn(shell, "Not initialized yet\n"));

	int status = ParseKeyId(shell, argv[1], keyId, numSlots);
	VerifyOrReturnStatus(status == 0, status);

	CryptoTypes::PublicKey publicKey{};

	const char *pubkeyStr{ argv[2] };
	size_t len = strlen(pubkeyStr);
	VerifyOrReturnStatus(len == kPublicKeyStringLength, -EINVAL, shell_warn(shell, "Invalid key length!\n"));

	len = hex2bin(pubkeyStr, len, publicKey.data(), publicKey.size());
	VerifyOrReturnStatus(len == publicKey.size(), -EINVAL, shell_warn(shell, "Invalid key length!\n"));

	VerifyOrReturnStatus(publicKey[0] == CryptoTypes::kEccP256PublicKeyPrefix, -EINVAL,
			     shell_warn(shell, "Invalid key prefix!\n"));

	if (FindPublicKeyInStorage(keyIdStr, publicKey, numSlots) != -ENOENT) {
		shell_warn(shell, "Key already exists!\n");
		return -EEXIST;
	}

	const auto publicKeyType = GetPublicKeyTypeFromStorageKey(keyIdStr);

	VerifyOrReturnStatus(RemovePublicKeyFromAccessManager(keyIdStr, keyId) == ALIRO_NO_ERROR, -EINVAL,
			     shell_warn(shell, "Cannot remove Access Credential public key from Access Manager\n"));

	const auto keyName = KeyValueStorage::GetStorageKeyName(keyIdStr, keyId);
	VerifyOrReturnStatus(!KeyValueStorage::Instance().Save(keyName.data(), publicKey.data(), publicKey.size()),
			     -EINVAL, shell_warn(shell, "Cannot save key-value in persistent storage!\n"));

	VerifyOrReturnStatus(AccessManagerInstance().AddPublicKey(publicKey, publicKeyType, keyId) == ALIRO_NO_ERROR,
			     -EINVAL, shell_warn(shell, "Cannot add Access Credential public key to Access Manager\n"));

	return 0;
}

int ShellCmdHandleAccessCredentialList(const struct shell *shell, size_t, char **)
{
	return CmdHandleAndListKey(shell, StorageKeys::kStorageKeyNameAccessCredentialPublicKey, kAcMaxKeys);
}

int ShellCmdHandleAccessCredentialSet(const struct shell *shell, size_t argc, char **argv)
{
	size_t keyId{};
	return CmdHandleAndSetKey(shell, argc, argv, StorageKeys::kStorageKeyNameAccessCredentialPublicKey, kAcMaxKeys,
				  keyId);
}

int ShellCmdHandleAccessCredentialClear(const struct shell *shell, size_t argc, char **argv)
{
	size_t keyId{};
	return CmdHandleAndClearKey(shell, argc, argv, StorageKeys::kStorageKeyNameAccessCredentialPublicKey,
				    kAcMaxKeys, keyId);
}

#if CONFIG_DOOR_LOCK_ACCESS_MANAGER_CREDENTIAL_ISSUER_MAX_STORED_KEYS > 0

int ShellCmdHandleCredentialIssuerList(const struct shell *shell, size_t, char **)
{
	return CmdHandleAndListKey(shell, StorageKeys::kStorageKeyNameCredentialIssuerPublicKey, kCiMaxKeys);
}

int ShellCmdHandleCredentialIssuerSet(const struct shell *shell, size_t argc, char **argv)
{
	size_t keyId{};
	return CmdHandleAndSetKey(shell, argc, argv, StorageKeys::kStorageKeyNameCredentialIssuerPublicKey, kCiMaxKeys,
				  keyId);
}

int ShellCmdHandleCredentialIssuerClear(const struct shell *shell, size_t argc, char **argv)
{
	size_t keyId{};
	const auto error = CmdHandleAndClearKey(
		shell, argc, argv, StorageKeys::kStorageKeyNameCredentialIssuerPublicKey, kCiMaxKeys, keyId);
	VerifyOrReturnValue(error == 0, error);

	VerifyOrReturnValue(ClearValidityIterations(keyId) == ALIRO_NO_ERROR, -EINVAL);

	return 0;
}

#endif // CONFIG_DOOR_LOCK_ACCESS_MANAGER_CREDENTIAL_ISSUER_MAX_STORED_KEYS > 0

#ifdef CONFIG_DOOR_LOCK_CREDENTIAL_ISSUER_CA

int ShellCmdHandleCredentialIssuerCAGet(const struct shell *shell, size_t argc, char **)
{
	VerifyOrReturnStatus(argc == 1, -EINVAL, shell_warn(shell, "Invalid number of arguments!\n"));
	VerifyOrReturnValue(isInitialized, -EIO, shell_warn(shell, "Not initialized yet\n"));

	CryptoTypes::PublicKey publicKey{};
	const auto error =
		CryptoInstance().ExportKey(kCredentialIssuerCAPublicKeyId, publicKey.data(), publicKey.size());
	VerifyOrReturnStatus(error == ALIRO_NO_ERROR, -EINVAL,
			     shell_warn(shell, "Cannot export Credential Issuer CA public key\n"));

	std::array<char, kPublicKeyStringLength + 1> hexString{};
	bin2hex(publicKey.data(), publicKey.size(), hexString.data(), hexString.size());
	shell_print(shell, "%s", hexString.data());

	return 0;
}

int ShellCmdHandleCredentialIssuerCASet(const struct shell *shell, size_t argc, char **argv)
{
	VerifyOrReturnStatus(argc == 2, -EINVAL, shell_warn(shell, "Invalid number of arguments!\n"));
	VerifyOrReturnValue(isInitialized, -EIO, shell_warn(shell, "Not initialized yet\n"));

	const char *pubkeyStr{ argv[1] };
	size_t len = strlen(pubkeyStr);
	VerifyOrReturnStatus(len == kPublicKeyStringLength, -EINVAL, shell_warn(shell, "Invalid key length!\n"));

	CryptoTypes::PublicKey publicKey{};
	const auto result = hex2bin(pubkeyStr, len, publicKey.data(), publicKey.size());
	VerifyOrReturnStatus(result == publicKey.size(), -EINVAL, shell_warn(shell, "Invalid key length!\n"));

	VerifyOrReturnStatus(publicKey[0] == CryptoTypes::kEccP256PublicKeyPrefix, -EINVAL,
			     shell_warn(shell, "Invalid key prefix!\n"));

	CryptoTypes::KeyId keyId{ kCredentialIssuerCAPublicKeyId };
	const auto error = CryptoInstance().ImportPublicKey(publicKey, keyId, true);
	VerifyOrReturnStatus(error == ALIRO_NO_ERROR, -EINVAL,
			     shell_warn(shell, "Cannot import Credential Issuer CA public key\n"));

	return 0;
}

int ShellCmdHandleCredentialIssuerCAClear(const struct shell *shell, size_t argc, char **argv)
{
	VerifyOrReturnStatus(argc == 1, -EINVAL, shell_warn(shell, "Invalid number of arguments!\n"));
	VerifyOrReturnValue(isInitialized, -EIO, shell_warn(shell, "Not initialized yet\n"));

	CryptoTypes::KeyId keyId{ kCredentialIssuerCAPublicKeyId };
	const auto error = CryptoInstance().DestroyKey(keyId);
	VerifyOrReturnStatus(error == ALIRO_NO_ERROR, -EINVAL,
			     shell_warn(shell, "Cannot remove Credential Issuer CA public key\n"));

	return 0;
}

#endif // CONFIG_DOOR_LOCK_CREDENTIAL_ISSUER_CA

int FactoryReset(const struct shell *shell, size_t, char **)
{
	VerifyOrReturnValue(isInitialized, -EIO, shell_warn(shell, "Not initialized yet\n"));

	void *storage{ nullptr };
	int status = settings_storage_get(&storage);
	VerifyOrReturnStatus(status == 0, -EIO, shell_warn(shell, "Cannot get storage\n"));

#ifdef CONFIG_SETTINGS_NVS
	status = nvs_clear(static_cast<nvs_fs *>(storage));
#else // CONFIG_SETTINGS_ZMS
	status = zms_clear(static_cast<zms_fs *>(storage));
#endif // CONFIG_SETTINGS_NVS || CONFIG_SETTINGS_ZMS

	VerifyOrReturnStatus(status == 0, -EIO, shell_warn(shell, "Cannot clear storage\n"));
	k_work_reschedule(&sRebootWork, K_MSEC(250));
	return 0;
}

#endif // CONFIG_CHIP

const char *GetReaderChipName(void)
{
#if defined(CONFIG_ST25R200_DRV)
	return "ST25R100";
#elif defined(CONFIG_ST25R500_DRV)
	return "ST25R300";
#else
	return "Unknown NFC reader driver";
#endif
}

int ShellCmdHandleInfo(const struct shell *shell, size_t, char **)
{
	shell_print(shell, "Aliro version: %s", AliroStack::GetLibraryVersion());
	shell_print(shell, "NFC reader: %s", GetReaderChipName());
	return 0;
}

#ifdef CONFIG_DOOR_LOCK_EXPEDITED_FAST_PHASE

int ParseIndex(const struct shell *shell, const char *indexStr, size_t &index)
{
	char *endPtr{ nullptr };
	errno = 0;
	unsigned long val = strtoul(indexStr, &endPtr, 10);
	VerifyOrReturnStatus(errno == 0 && endPtr != indexStr, -EINVAL, shell_warn(shell, "Invalid index!\n"));

	index = val;

	return 0;
}

int PrintKpersistentKeys(const struct shell *shell, CryptoTypes::KeyId *kpersistentKeyIds, size_t kpersistentCount)
{
	shell_print(shell, "Number of Kpersistent keys: %u", kpersistentCount);
	shell_print(shell, "Index   ID          Public Key");
	shell_print(shell, "--------------------------------");
	for (size_t i = 0; i < kpersistentCount; i++) {
		const CryptoTypes::KeyId kpersistentKeyId = kpersistentKeyIds[i];
		const size_t kpersistentKeyIndex = kpersistentKeyId - kKpersistentRangeBegin;

		CryptoTypes::PublicKey publicKey{};
		VerifyOrReturnValue(sKpersistentManager->GetAccessCredentialPublicKey(kpersistentKeyId, publicKey) ==
					    ALIRO_NO_ERROR,
				    -EINVAL, shell_warn(shell, "Cannot get Access Credential public key\n"));
		std::array<char, kPublicKeyStringLength + 1> hexString{};
		bin2hex(publicKey.data(), publicKey.size(), hexString.data(), hexString.size());
		shell_print(shell, "%-2u      0x%08x  %s", kpersistentKeyIndex, kpersistentKeyId, hexString.data());
	}

	return 0;
}

int ShellCmdHandleKpersistentList(const struct shell *shell, size_t argc, char **argv)
{
	VerifyOrReturnValue(isInitialized, -EIO, shell_warn(shell, "Not initialized yet\n"));
	VerifyOrReturnValue(sKpersistentManager, -EIO, shell_warn(shell, "Kpersistent manager not initialized\n"));
	VerifyOrReturnValue(argc == 1, -EINVAL, shell_warn(shell, "Invalid number of arguments!\n"));

	constexpr char kList[] = "list";
	VerifyOrReturnValue(CmdMatch(argv[0], kList), -EINVAL, shell_warn(shell, "Invalid command!\n"));

	size_t kpersistentCount{};
	VerifyOrReturnValue(sKpersistentManager->GetKpersistentCount(kpersistentCount) == ALIRO_NO_ERROR, -EINVAL,
			    shell_warn(shell, "Cannot get Kpersistent count\n"));
	VerifyOrReturnValue(kpersistentCount > 0, 0, shell_print(shell, "No Kpersistent keys found\n"));

	auto kpersistentKeyIds = Aliro::make_unique_array_nothrow<CryptoTypes::KeyId>(kpersistentCount);
	VerifyOrReturnValue(kpersistentKeyIds, -ENOMEM,
			    shell_warn(shell, "Cannot allocate memory for Kpersistent key IDs\n"));

	VerifyOrReturnValue(sKpersistentManager->GetKpersistentKeyIds(kpersistentKeyIds.get(), kpersistentCount) ==
				    ALIRO_NO_ERROR,
			    -EINVAL, shell_warn(shell, "Cannot get Kpersistent key IDs\n"));

	return PrintKpersistentKeys(shell, kpersistentKeyIds.get(), kpersistentCount);
}

int ShellCmdHandleKpersistentClear(const struct shell *shell, size_t argc, char **argv)
{
	VerifyOrReturnValue(isInitialized, -EIO, shell_warn(shell, "Not initialized yet\n"));
	VerifyOrReturnValue(sKpersistentManager, -EIO, shell_warn(shell, "Kpersistent manager not initialized\n"));
	VerifyOrReturnValue(argc == 2, -EINVAL, shell_warn(shell, "Invalid number of arguments!\n"));

	constexpr char kRemove[] = "clear";
	VerifyOrReturnValue(CmdMatch(argv[0], kRemove), -EINVAL, shell_warn(shell, "Invalid command!\n"));

	constexpr char kAll[] = "all";
	if (strncmp(argv[1], kAll, CStrLen(kAll)) == 0) {
		shell_print(shell, "Removing all Kpersistent keys");
		sKpersistentManager->RemoveAllKpersistent();
		return 0;
	}

	size_t index{};
	VerifyOrReturnValue(ParseIndex(shell, argv[1], index) == 0, -EINVAL);
	shell_print(shell, "Removing Kpersistent key with index: %u", index);
	VerifyOrReturnValue(sKpersistentManager->RemoveKpersistent(index) == ALIRO_NO_ERROR, -EINVAL,
			    shell_warn(shell, "Cannot remove Kpersistent key\n"));
	return 0;
}

#endif // CONFIG_DOOR_LOCK_EXPEDITED_FAST_PHASE

#ifdef CONFIG_DOOR_LOCK_READER_CERTIFICATE

int ShellCmdHandleReaderCertList(const struct shell *shell, size_t, char **)
{
	VerifyOrReturnValue(isInitialized, -EIO, shell_warn(shell, "Not initialized yet\n"));

	ConstData cert{};
	const auto error = ReaderCertificateCache::Instance().GetCertificate(cert);
	if (error == ALIRO_NO_ERROR) {
		std::array<char, StorageKeys::kMaxCertificateSize * 2 + 1> hexString{ 0 };
		size_t len = bin2hex(cert.mData, cert.mLength, hexString.data(), hexString.size());
		VerifyOrReturnStatus(len == cert.mLength * 2, -EINVAL,
				     shell_warn(shell, "Invalid certificate hex string!\n"));
		shell_print(shell, "Reader certificate (%zu bytes): %s", cert.mLength, hexString.data());
	} else {
		shell_print(shell, "Reader certificate: (not set)");
	}

	return 0;
}

int ShellCmdHandleReaderCertSet(const struct shell *shell, size_t argc, char **argv)
{
	VerifyOrReturnStatus(IN_RANGE(argc, 2, 2), -EINVAL, shell_warn(shell, "Invalid number of arguments!\n"));
	VerifyOrReturnValue(isInitialized, -EIO, shell_warn(shell, "Not initialized yet\n"));

	const char *certStr = argv[1];
	size_t certStrLen = strlen(certStr);

	VerifyOrReturnStatus(certStrLen > 0 && certStrLen % 2 == 0, -EINVAL,
			     shell_warn(shell, "Invalid certificate length (must be even)!\n"));

	std::array<uint8_t, StorageKeys::kMaxCertificateSize> certData{ 0 };
	size_t decodedLen = hex2bin(certStr, certStrLen, certData.data(), certData.size());
	VerifyOrReturnStatus(decodedLen == certStrLen / 2, -EINVAL,
			     shell_warn(shell, "Invalid certificate hex string!\n"));

	// save to cache
	AliroError error = ReaderCertificateCache::Instance().SetCertificate({ certData.data(), decodedLen });
	VerifyOrReturnStatus(error == ALIRO_NO_ERROR, -EINVAL,
			     shell_warn(shell, "Failed to set certificate: %d\n", error.ToInt()));

	// save certificate data to persistent storage
	VerifyOrReturnStatus(!KeyValueStorage::Instance().Save(StorageKeys::kStorageKeyNameReaderCertificate,
							       certData.data(), decodedLen),
			     -EINVAL, shell_warn(shell, "Cannot save certificate to persistent storage!\n"));

	// save certificate length to persistent storage
	uint16_t certLength = static_cast<uint16_t>(decodedLen);
	VerifyOrReturnStatus(!KeyValueStorage::Instance().Save(StorageKeys::kStorageKeyNameReaderCertificateLength,
							       reinterpret_cast<const uint8_t *>(&certLength),
							       sizeof(certLength)),
			     -EINVAL, shell_warn(shell, "Cannot save certificate length to persistent storage!\n"));

	shell_print(shell, "Reader certificate set successfully (%u bytes)", certLength);
	return 0;
}

int ShellCmdHandleReaderCertClear(const struct shell *shell, size_t argc, char **)
{
	VerifyOrReturnStatus(IN_RANGE(argc, 1, 1), -EINVAL, shell_warn(shell, "Invalid number of arguments!\n"));
	VerifyOrReturnValue(isInitialized, -EIO, shell_warn(shell, "Not initialized yet\n"));

	// clear cache
	ReaderCertificateCache::Instance().ClearCertificate();

	// clear certificate data from persistent storage
	VerifyOrReturnStatus(!KeyValueStorage::Instance().Clear(StorageKeys::kStorageKeyNameReaderCertificate), -EIO,
			     shell_warn(shell, "Cannot clear certificate from persistent storage!\n"));

	// clear certificate length from persistent storage
	VerifyOrReturnStatus(!KeyValueStorage::Instance().Clear(StorageKeys::kStorageKeyNameReaderCertificateLength),
			     -EIO, shell_warn(shell, "Cannot clear certificate length from persistent storage!\n"));

	shell_print(shell, "Reader certificate cleared successfully");
	return 0;
}

int ShellCmdHandleIssuerPublicKeyList(const struct shell *shell, size_t, char **)
{
	VerifyOrReturnValue(isInitialized, -EIO, shell_warn(shell, "Shell not initialized\n"));

	CryptoTypes::PublicKey publicKey{};
	const auto error = ReaderCertificateCache::Instance().GetIssuerPublicKey(publicKey);
	if (error == ALIRO_NO_ERROR) {
		std::array<char, CryptoTypes::kEccP256PublicKeyLength * 2 + 1> hexString{ 0 };
		size_t len = bin2hex(publicKey.data(), publicKey.size(), hexString.data(), hexString.size());
		VerifyOrReturnStatus(len == publicKey.size() * 2, -EINVAL,
				     shell_warn(shell, "Invalid public key hex string!\n"));
		shell_print(shell, "Issuer public key (%zu bytes): %s", publicKey.size(), hexString.data());
	} else {
		shell_print(shell, "Issuer public key: (not set)");
	}

	return 0;
}

int ShellCmdHandleIssuerPublicKeySet(const struct shell *shell, size_t argc, char **argv)
{
	VerifyOrReturnStatus(IN_RANGE(argc, 2, 2), -EINVAL, shell_warn(shell, "Invalid number of arguments!\n"));
	VerifyOrReturnValue(isInitialized, -EIO, shell_warn(shell, "Shell not initialized\n"));

	const char *keyStr = argv[1];
	size_t keyStrLen = strlen(keyStr);

	VerifyOrReturnStatus(keyStrLen == CryptoTypes::kEccP256PublicKeyLength * 2, -EINVAL,
			     shell_warn(shell, "Invalid key length (must be %zu hex characters)!\n",
					CryptoTypes::kEccP256PublicKeyLength * 2));

	CryptoTypes::PublicKey publicKey{};
	size_t decodedLen = hex2bin(keyStr, keyStrLen, publicKey.data(), publicKey.size());
	VerifyOrReturnStatus(decodedLen == publicKey.size(), -EINVAL,
			     shell_warn(shell, "Invalid public key hex string!\n"));

	VerifyOrReturnStatus(publicKey[0] == CryptoTypes::kEccP256PublicKeyPrefix, -EINVAL,
			     shell_warn(shell, "Invalid key prefix (must be 0x04)!\n"));

	// save to persistent storage
	VerifyOrReturnStatus(
		!KeyValueStorage::Instance().Save(StorageKeys::kStorageKeyNameReaderSystemIssuerCAPublicKey,
						  publicKey.data(), publicKey.size()),
		-EINVAL, shell_warn(shell, "Cannot save issuer public key to persistent storage!\n"));

	// save to cache
	VerifyOrReturnStatus(ReaderCertificateCache::Instance().SetIssuerPublicKey(publicKey) == ALIRO_NO_ERROR,
			     -EINVAL, shell_warn(shell, "Failed to set issuer public key!\n"));

	shell_print(shell, "Issuer public key set successfully (%zu bytes)", publicKey.size());

	return 0;
}

int ShellCmdHandleIssuerPublicKeyClear(const struct shell *shell, size_t argc, char **)
{
	VerifyOrReturnStatus(IN_RANGE(argc, 1, 1), -EINVAL, shell_warn(shell, "Invalid number of arguments!\n"));
	VerifyOrReturnValue(isInitialized, -EIO, shell_warn(shell, "Shell not initialized\n"));

	// clear from persistent storage
	VerifyOrReturnStatus(
		!KeyValueStorage::Instance().Clear(StorageKeys::kStorageKeyNameReaderSystemIssuerCAPublicKey), -EIO,
		shell_warn(shell, "Cannot clear issuer public key from persistent storage!\n"));

	// clear from cache
	ReaderCertificateCache::Instance().ClearIssuerPublicKey();

	shell_print(shell, "Issuer public key cleared successfully");
	return 0;
}

#endif // CONFIG_DOOR_LOCK_READER_CERTIFICATE

} // namespace

void InitShellCommands([[maybe_unused]] Aliro::KpersistentManager *kpersistentManager)
{
#ifdef CONFIG_DOOR_LOCK_EXPEDITED_FAST_PHASE
	sKpersistentManager = kpersistentManager;
#endif // CONFIG_DOOR_LOCK_EXPEDITED_FAST_PHASE

	isInitialized = true;
}

#ifndef CONFIG_CHIP

SHELL_STATIC_SUBCMD_SET_CREATE(
	install_cmd,
	SHELL_CMD(identifier, NULL,
		  "Set or get reader identifier\n"
		  "  Usage: dl install identifier <32-byte reader_identifier in hex without 0x>",
		  ShellCmdHandleIdentifiers),
	SHELL_CMD(group_id, NULL,
		  "Set or get group ID\n"
		  "  Usage: dl install group_id <16-byte reader_group_identifier in hex without 0x>",
		  ShellCmdHandleIdentifiers),
	SHELL_CMD(group_sub_id, NULL,
		  "Set or get group sub ID\n"
		  "  Usage: dl install group_sub_id <16-byte reader_group_sub_identifier in hex without 0x>",
		  ShellCmdHandleIdentifiers),
	SHELL_SUBCMD_SET_END);

SHELL_STATIC_SUBCMD_SET_CREATE(AC_key_cmd,
			       SHELL_CMD(list, NULL,
					 "List Access Credential public keys\n"
					 "  Usage: dl AC_key list",
					 ShellCmdHandleAccessCredentialList),
			       SHELL_CMD(set, NULL,
					 "Set Access Credential public key\n"
					 "  Usage: dl AC_key set <key_id> <65-byte public key in hex without 0x>",
					 ShellCmdHandleAccessCredentialSet),
			       SHELL_CMD(clear, NULL,
					 "Clear Access Credential public key\n"
					 "  Usage: dl AC_key clear <key_id>\n"
					 "         dl AC_key clear all\n",
					 ShellCmdHandleAccessCredentialClear),
			       SHELL_SUBCMD_SET_END);

#if CONFIG_DOOR_LOCK_ACCESS_MANAGER_CREDENTIAL_ISSUER_MAX_STORED_KEYS > 0

SHELL_STATIC_SUBCMD_SET_CREATE(CI_key_cmd,
			       SHELL_CMD(list, NULL,
					 "List Credential Issuer public keys\n"
					 "  Usage: dl CI_key list",
					 ShellCmdHandleCredentialIssuerList),
			       SHELL_CMD(set, NULL,
					 "Set Credential Issuer public key\n"
					 "  Usage: dl CI_key set <key_id> <65-byte public key in hex without 0x>",
					 ShellCmdHandleCredentialIssuerSet),
			       SHELL_CMD(clear, NULL,
					 "Clear Credential Issuer public key\n"
					 "  Usage: dl CI_key clear <key_id>\n"
					 "         dl CI_key clear all\n",
					 ShellCmdHandleCredentialIssuerClear),
			       SHELL_SUBCMD_SET_END);

#endif // CONFIG_DOOR_LOCK_ACCESS_MANAGER_CREDENTIAL_ISSUER_MAX_STORED_KEYS > 0

#ifdef CONFIG_DOOR_LOCK_CREDENTIAL_ISSUER_CA

SHELL_STATIC_SUBCMD_SET_CREATE(CI_CA_key_cmd,
			       SHELL_CMD(get, NULL,
					 "Get Credential Issuer CA public key\n"
					 "  Usage: dl CI_CA_key get",
					 ShellCmdHandleCredentialIssuerCAGet),
			       SHELL_CMD(set, NULL,
					 "Set Credential Issuer CA public key\n"
					 "  Usage: dl CI_CA_key set <65-byte public key in hex without 0x>",
					 ShellCmdHandleCredentialIssuerCASet),
			       SHELL_CMD(clear, NULL,
					 "Clear Credential Issuer CA public key\n"
					 "  Usage: dl CI_CA_key clear\n",
					 ShellCmdHandleCredentialIssuerCAClear),
			       SHELL_SUBCMD_SET_END);

#endif // CONFIG_DOOR_LOCK_CREDENTIAL_ISSUER_CA

#ifdef CONFIG_DOOR_LOCK_READER_CERTIFICATE

SHELL_STATIC_SUBCMD_SET_CREATE(rcert_cmd,
			       SHELL_CMD(list, NULL,
					 "List Reader certificate\n"
					 "  Usage: dl reader_cert list",
					 ShellCmdHandleReaderCertList),
			       SHELL_CMD(set, NULL,
					 "Set Reader certificate (compressed)\n"
					 "  Usage: dl reader_cert set <certificate in hex without 0x>",
					 ShellCmdHandleReaderCertSet),
			       SHELL_CMD(clear, NULL,
					 "Clear Reader certificate\n"
					 "  Usage: dl reader_cert clear",
					 ShellCmdHandleReaderCertClear),
			       SHELL_SUBCMD_SET_END);

SHELL_STATIC_SUBCMD_SET_CREATE(issuer_pk_cmd,
			       SHELL_CMD(list, NULL,
					 "List Issuer public key\n"
					 "  Usage: dl issuer_pk list",
					 ShellCmdHandleIssuerPublicKeyList),
			       SHELL_CMD(set, NULL,
					 "Set Issuer public key (65 bytes)\n"
					 "  Usage: dl issuer_pk set <public key in hex without 0x>",
					 ShellCmdHandleIssuerPublicKeySet),
			       SHELL_CMD(clear, NULL,
					 "Clear Issuer public key\n"
					 "  Usage: dl issuer_pk clear",
					 ShellCmdHandleIssuerPublicKeyClear),
			       SHELL_SUBCMD_SET_END);

#endif // CONFIG_DOOR_LOCK_READER_CERTIFICATE

SHELL_STATIC_SUBCMD_SET_CREATE(provisioning_cmd,
			       SHELL_CMD(AC_key, &AC_key_cmd, "Manage Access Credential public keys", NULL),

#if CONFIG_DOOR_LOCK_ACCESS_MANAGER_CREDENTIAL_ISSUER_MAX_STORED_KEYS > 0
			       SHELL_CMD(CI_key, &CI_key_cmd, "Manage Credential Issuer public keys", NULL),
#endif // CONFIG_DOOR_LOCK_ACCESS_MANAGER_CREDENTIAL_ISSUER_MAX_STORED_KEYS > 0

#ifdef CONFIG_DOOR_LOCK_CREDENTIAL_ISSUER_CA
			       SHELL_CMD(CI_CA_key, &CI_CA_key_cmd, "Manage Credential Issuer CA public key", NULL),
#endif // CONFIG_DOOR_LOCK_CREDENTIAL_ISSUER_CA

#ifdef CONFIG_DOOR_LOCK_READER_CERTIFICATE
			       SHELL_CMD(reader_cert, &rcert_cmd, "Manage Reader certificate", NULL),
			       SHELL_CMD(issuer_pk, &issuer_pk_cmd, "Manage Issuer public key", NULL),
#endif // CONFIG_DOOR_LOCK_READER_CERTIFICATE

			       SHELL_SUBCMD_SET_END);

#endif // CONFIG_CHIP

#ifdef CONFIG_DOOR_LOCK_EXPEDITED_FAST_PHASE

SHELL_STATIC_SUBCMD_SET_CREATE(kpersistent_cmd,
			       SHELL_CMD(list, NULL,
					 "List Kpersistent keys\n"
					 "  Usage: dl kpersistent list",
					 ShellCmdHandleKpersistentList),
			       SHELL_CMD(clear, NULL,
					 "Clear Kpersistent key\n"
					 "  Usage: dl kpersistent clear <index>\n"
					 "       dl kpersistent clear all",
					 ShellCmdHandleKpersistentClear),
			       SHELL_SUBCMD_SET_END);

#endif // CONFIG_DOOR_LOCK_EXPEDITED_FAST_PHASE

SHELL_STATIC_SUBCMD_SET_CREATE(door_lock_cmd,

			       SHELL_CMD(info, NULL, "Show Aliro lib version and NFC reader chip name",
					 ShellCmdHandleInfo),

#ifdef CONFIG_DOOR_LOCK_EXPEDITED_FAST_PHASE

			       SHELL_CMD(kpersistent, &kpersistent_cmd, "Manage Kpersistent keys", NULL),

#endif // CONFIG_DOOR_LOCK_EXPEDITED_FAST_PHASE

#ifndef CONFIG_CHIP

#ifdef CONFIG_DOOR_LOCK_DFU_BLE_SMP_STANDALONE
			       SHELL_CMD(dfu_smp, NULL, "Enable/disable DFU BLE SMP: dl dfu_smp <on|off>",
					 ShellCmdHandleDfuSmp),
#endif // CONFIG_DOOR_LOCK_DFU_BLE_SMP_STANDALONE

			       SHELL_CMD(install, &install_cmd, "Installation commands", NULL),
			       SHELL_CMD(provisioning, &provisioning_cmd, "Provisioning commands", NULL),
			       SHELL_CMD(factory_reset, NULL, "Factory reset", FactoryReset),

#endif // CONFIG_CHIP

			       SHELL_SUBCMD_SET_END);

SHELL_CMD_REGISTER(dl, &door_lock_cmd, "Door lock commands", NULL);
