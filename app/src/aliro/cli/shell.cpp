/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "shell.h"

#include <aliro/aliro.h>
#include <aliro/utils.h>

#include <zephyr/shell/shell.h>

#if defined(CONFIG_DOOR_LOCK_DFU_BLE_SMP) && !defined(CONFIG_ALIRO_BLE_UWB) && !defined(CONFIG_DOOR_LOCK_BLE_NUS)

#include "dfu_smp_shell.h"

#endif // CONFIG_DOOR_LOCK_DFU_BLE_SMP && !CONFIG_ALIRO_BLE_UWB && !CONFIG_DOOR_LOCK_BLE_NUS

#ifndef CONFIG_CHIP

#include "access_manager/access_manager.h"
#include "storage.h"
#include "storage_keys.h"

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

// Flag to check if shell is already initialized.
bool isInitialized{ false };

#ifndef CONFIG_CHIP

using PublicKeyType = AccessManager::PublicKeyType;

constexpr char kCmdReaderIdentifier[] = "identifier";
constexpr char kCmdReaderGroupIdentifier[] = "group_id";
constexpr char kCmdReaderGroupSubIdentifier[] = "group_sub_id";
constexpr size_t kPublicKeyStringLength{ 2 * CryptoTypes::kEccP256PublicKeyLength };
constexpr size_t kAcMaxKeys{ CONFIG_ALIRO_ACCESS_MANAGER_MAX_STORED_KEYS };
[[maybe_unused]] constexpr size_t kCiMaxKeys{ CONFIG_ALIRO_CREDENTIAL_ISSUER_MAX_STORED_KEYS };

K_WORK_DELAYABLE_DEFINE(sRebootWork, [](k_work *) { sys_reboot(SYS_REBOOT_WARM); });

template <size_t N> constexpr size_t CStrLen(const char (&)[N])
{
	return N;
}

template <size_t N> bool CmdMatch(const char *cmd, const char (&cmdStr)[N])
{
	return strncmp(cmd, cmdStr, CStrLen(cmdStr)) == 0;
}

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

	AliroStack::Instance().SetReaderIdentifier(identifier);

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
		ReturnErrorOnFailure(AccessManagerInstance().RemovePublicKey(publicKey, publicKeyType));
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

	for (size_t keyId = 0; keyId < numSlots; keyId++) {
		std::array<char, kPublicKeyStringLength * 2 + 1> hexString{};
		CryptoTypes::PublicKey publicKey{};

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
			 size_t numSlots)
{
	VerifyOrReturnStatus(IN_RANGE(argc, 2, 2), -EINVAL, shell_warn(shell, "Invalid number of arguments!\n"));
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

	size_t keyId{};
	int status = ParseKeyId(shell, argv[1], keyId, numSlots);
	VerifyOrReturnStatus(status == 0, status);

	status = RemoveKey(shell, keyIdStr, keyId);
	VerifyOrReturnStatus(status == 0, status);
	return 0;
}

int CmdHandleAndSetKey(const struct shell *shell, size_t argc, char **argv, KeyValueStorage::KeyIdString keyIdStr,
		       size_t numSlots)
{
	VerifyOrReturnStatus(IN_RANGE(argc, 3, 3), -EINVAL, shell_warn(shell, "Invalid number of arguments!\n"));
	VerifyOrReturnValue(isInitialized, -EIO, shell_warn(shell, "Not initialized yet\n"));

	size_t keyId{};
	int status = ParseKeyId(shell, argv[1], keyId, numSlots);
	VerifyOrReturnStatus(status == 0, status);

	CryptoTypes::PublicKey publicKey{};

	const char *pubkeyStr{ argv[2] };
	size_t len = strlen(pubkeyStr);
	VerifyOrReturnStatus(len == kPublicKeyStringLength, -EINVAL, shell_warn(shell, "Invalid key length!\n"));

	len = hex2bin(pubkeyStr, len, publicKey.data(), publicKey.size());
	VerifyOrReturnStatus(len == publicKey.size(), -EINVAL, shell_warn(shell, "Invalid key value!\n"));

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

	VerifyOrReturnStatus(AccessManagerInstance().AddPublicKey(publicKey, publicKeyType) == ALIRO_NO_ERROR, -EINVAL,
			     shell_warn(shell, "Cannot add Access Credential public key to Access Manager\n"));

	return 0;
}

int ShellCmdHandleAccessCredentialList(const struct shell *shell, size_t, char **)
{
	return CmdHandleAndListKey(shell, StorageKeys::kStorageKeyNameAccessCredentialPublicKey, kAcMaxKeys);
}

int ShellCmdHandleAccessCredentialSet(const struct shell *shell, size_t argc, char **argv)
{
	return CmdHandleAndSetKey(shell, argc, argv, StorageKeys::kStorageKeyNameAccessCredentialPublicKey, kAcMaxKeys);
}

int ShellCmdHandleAccessCredentialClear(const struct shell *shell, size_t argc, char **argv)
{
	return CmdHandleAndClearKey(shell, argc, argv, StorageKeys::kStorageKeyNameAccessCredentialPublicKey,
				    kAcMaxKeys);
}

#if CONFIG_ALIRO_CREDENTIAL_ISSUER_MAX_STORED_KEYS > 0

int ShellCmdHandleCredentialIssuerList(const struct shell *shell, size_t, char **)
{
	return CmdHandleAndListKey(shell, StorageKeys::kStorageKeyNameIssuerCredentialPublicKey, kCiMaxKeys);
}

int ShellCmdHandleCredentialIssuerSet(const struct shell *shell, size_t argc, char **argv)
{
	return CmdHandleAndSetKey(shell, argc, argv, StorageKeys::kStorageKeyNameIssuerCredentialPublicKey, kCiMaxKeys);
}

int ShellCmdHandleCredentialIssuerClear(const struct shell *shell, size_t argc, char **argv)
{
	return CmdHandleAndClearKey(shell, argc, argv, StorageKeys::kStorageKeyNameIssuerCredentialPublicKey,
				    kCiMaxKeys);
}

#endif // CONFIG_ALIRO_CREDENTIAL_ISSUER_MAX_STORED_KEYS > 0

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
#elif defined(CONFIG_ST25R3911_DRV)
	return "ST25R3911";
#elif defined(CONFIG_ST25R3916_DRV)
	return "ST25R3916";
#elif defined(CONFIG_ST25R3916B_DRV)
	return "ST25R3916B";
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

} // namespace

void InitShellCommands()
{
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

#if CONFIG_ALIRO_CREDENTIAL_ISSUER_MAX_STORED_KEYS > 0

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

#endif // CONFIG_ALIRO_CREDENTIAL_ISSUER_MAX_STORED_KEYS > 0

SHELL_STATIC_SUBCMD_SET_CREATE(provisioning_cmd,
			       SHELL_CMD(AC_key, &AC_key_cmd, "Manage Access Credential public keys", NULL),

#if CONFIG_ALIRO_CREDENTIAL_ISSUER_MAX_STORED_KEYS > 0
			       SHELL_CMD(CI_key, &CI_key_cmd, "Manage Credential Issuer public keys", NULL),
#endif // CONFIG_ALIRO_CREDENTIAL_ISSUER_MAX_STORED_KEYS > 0

			       SHELL_SUBCMD_SET_END);

#endif // CONFIG_CHIP

SHELL_STATIC_SUBCMD_SET_CREATE(
	door_lock_cmd, SHELL_CMD(info, NULL, "Show Aliro lib version and NFC reader chip name", ShellCmdHandleInfo),

#ifndef CONFIG_CHIP

#if defined(CONFIG_DOOR_LOCK_DFU_BLE_SMP) && !defined(CONFIG_ALIRO_BLE_UWB) && !defined(CONFIG_DOOR_LOCK_BLE_NUS)

	SHELL_CMD(dfu_smp, NULL, "Enable/disable DFU BLE SMP: dl dfu_smp <on|off>", ShellCmdHandleDfuSmp),

#endif // CONFIG_DOOR_LOCK_DFU_BLE_SMP && !CONFIG_ALIRO_BLE_UWB && !CONFIG_DOOR_LOCK_BLE_NUS

	SHELL_CMD(install, &install_cmd, "Installation commands", NULL),
	SHELL_CMD(provisioning, &provisioning_cmd, "Provisioning commands", NULL),
	SHELL_CMD(factory_reset, NULL, "Factory reset", FactoryReset),

#endif // CONFIG_CHIP

	SHELL_SUBCMD_SET_END);

SHELL_CMD_REGISTER(dl, &door_lock_cmd, "Door lock commands", NULL);
