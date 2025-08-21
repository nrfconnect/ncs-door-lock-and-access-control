/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "shell.h"

#include <aliro/aliro.h>
#include <aliro/utils.h>

#include "access_manager/access_manager.h"
#include "storage.h"
#include "storage_keys.h"

#include <zephyr/fs/nvs.h>
#include <zephyr/settings/settings.h>
#include <zephyr/shell/shell.h>
#include <zephyr/sys/reboot.h>
#include <zephyr/sys/util.h>

#include <array>

namespace {
using namespace Aliro;

constexpr char kCmdReaderIdentifier[] = "identifier";
constexpr char kCmdReaderGroupIdentifier[] = "group_id";
constexpr char kCmdReaderGroupSubIdentifier[] = "group_sub_id";

constexpr size_t kAccessCredentialPublicKeyStringLength{ 2 * CryptoTypes::kEccP256PublicKeyLength };

k_work_delayable sRebootWork{};

template <size_t N> constexpr size_t CStrLen(const char (&)[N])
{
	return N;
}

template <size_t N> bool CmdMatch(const char *cmd, const char (&cmdStr)[N])
{
	return strncmp(cmd, cmdStr, CStrLen(cmdStr)) == 0;
}

int ShellCmdHandleIdentifiers(const struct shell *shell, size_t argc, char **argv)
{
	VerifyOrReturnStatus(IN_RANGE(argc, 1, 2), -EINVAL, shell_warn(shell, "Invalid number of arguments!\n"));

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

void InitACKeyKeyName(size_t keyId, StorageKeys::KeyNameBuffer &keyName)
{
	snprintf(keyName.data(), keyName.size(), "%s/%zu", StorageKeys::kStorageKeyNameAccessCredentialPublicKey,
		 keyId);
}

int ReadACKey(size_t keyId, CryptoTypes::PublicKey &pubKey)
{
	StorageKeys::KeyNameBuffer keyName;
	InitACKeyKeyName(keyId, keyName);

	return KeyValueStorage::Instance().Get(keyName.data(), pubKey.data(), pubKey.size());
}

AliroError RemoveACKeyFromAccessManager(size_t keyId)
{
	CryptoTypes::PublicKey accessCredentialPublicKey{};
	if (ReadACKey(keyId, accessCredentialPublicKey) == 0) {
		ReturnErrorOnFailure(AccessManagerInstance().RemovePublicKey(accessCredentialPublicKey));
	}

	return ALIRO_NO_ERROR;
}

int RemoveACKey(const struct shell *shell, size_t keyId)
{
	VerifyOrReturnStatus(RemoveACKeyFromAccessManager(keyId) == ALIRO_NO_ERROR, -EINVAL,
			     shell_warn(shell, "Cannot remove Access Credential public key from Access Manager\n"));

	StorageKeys::KeyNameBuffer keyName;
	InitACKeyKeyName(keyId, keyName);
	VerifyOrReturnStatus(!KeyValueStorage::Instance().Clear(keyName.data()), -EIO,
			     shell_warn(shell, "Cannot clear key-value in persistent storage!\n"));

	return 0;
}

int FindACKey(const CryptoTypes::PublicKey &pubKey)
{
	const size_t acSlots = AliroStack::Instance().GetConfig().mAccessCredentialKeySlots;
	for (size_t keyId = 0; keyId < acSlots; keyId++) {
		CryptoTypes::PublicKey accessCredentialPublicKey{};
		if (ReadACKey(keyId, accessCredentialPublicKey) == 0 && accessCredentialPublicKey == pubKey) {
			return keyId;
		}
	}

	return -ENOENT;
}

int ParseKeyId(const struct shell *shell, const char *const keyIdStr, size_t &keyId)
{
	char *endPtr{ nullptr };
	errno = 0;
	unsigned long val = strtoul(keyIdStr, &endPtr, 10);
	VerifyOrReturnStatus(errno == 0 && endPtr != keyIdStr, -EINVAL, shell_warn(shell, "Invalid key ID!\n"));

	const size_t acSlots = AliroStack::Instance().GetConfig().mAccessCredentialKeySlots;
	VerifyOrReturnStatus(acSlots > 0, -EINVAL,
			     shell_warn(shell, "Access Credential public key slots not supported!\n"));

	VerifyOrReturnStatus(IN_RANGE(val, 0, acSlots - 1), -EINVAL,
			     shell_warn(shell, "Key ID out of range, must be between 0 and %zu!\n", acSlots - 1));

	keyId = val;
	return 0;
}

int ShellCmdHandleAccessCredentialList(const struct shell *shell, size_t, char **)
{
	const size_t acSlots = AliroStack::Instance().GetConfig().mAccessCredentialKeySlots;
	for (size_t keyId = 0; keyId < acSlots; keyId++) {
		std::array<char, kAccessCredentialPublicKeyStringLength * 2 + 1> hexString{};
		CryptoTypes::PublicKey accessCredentialPublicKey{};

		if (ReadACKey(keyId, accessCredentialPublicKey) == 0) {
			bin2hex(accessCredentialPublicKey.data(), accessCredentialPublicKey.size(), hexString.data(),
				hexString.size());
		} else {
			snprintf(hexString.data(), hexString.size(), "(null)");
		}

		shell_print(shell, "[%zu]: %s", keyId, hexString.data());
	}

	return 0;
}

int ShellCmdHandleAccessCredentialSet(const struct shell *shell, size_t argc, char **argv)
{
	VerifyOrReturnStatus(IN_RANGE(argc, 3, 3), -EINVAL, shell_warn(shell, "Invalid number of arguments!\n"));

	size_t keyId{};
	int status = ParseKeyId(shell, argv[1], keyId);
	VerifyOrReturnStatus(status == 0, status);

	CryptoTypes::PublicKey accessCredentialPublicKey{};

	const char *acStr{ argv[2] };
	size_t len = strlen(acStr);
	VerifyOrReturnStatus(len == kAccessCredentialPublicKeyStringLength, -EINVAL,
			     shell_warn(shell, "Invalid key length!\n"));

	len = hex2bin(acStr, len, accessCredentialPublicKey.data(), accessCredentialPublicKey.size());
	VerifyOrReturnStatus(len == accessCredentialPublicKey.size(), -EINVAL,
			     shell_warn(shell, "Invalid key value!\n"));

	VerifyOrReturnStatus(accessCredentialPublicKey[0] == CryptoTypes::kEccP256PublicKeyPrefix, -EINVAL,
			     shell_warn(shell, "Invalid key prefix!\n"));

	if (FindACKey(accessCredentialPublicKey) != -ENOENT) {
		shell_warn(shell, "Key already exists!\n");
		return -EEXIST;
	}

	VerifyOrReturnStatus(RemoveACKeyFromAccessManager(keyId) == ALIRO_NO_ERROR, -EINVAL,
			     shell_warn(shell, "Cannot remove Access Credential public key from Access Manager\n"));

	StorageKeys::KeyNameBuffer keyName;
	InitACKeyKeyName(keyId, keyName);
	VerifyOrReturnStatus(!KeyValueStorage::Instance().Save(keyName.data(), accessCredentialPublicKey.data(),
							       accessCredentialPublicKey.size()),
			     -EINVAL, shell_warn(shell, "Cannot save key-value in persistent storage!\n"));

	VerifyOrReturnStatus(AccessManagerInstance().AddPublicKey(accessCredentialPublicKey) == ALIRO_NO_ERROR, -EINVAL,
			     shell_warn(shell, "Cannot add Access Credential public key to Access Manager\n"));

	return 0;
}

int ShellCmdHandleAccessCredentialClear(const struct shell *shell, size_t argc, char **argv)
{
	VerifyOrReturnStatus(IN_RANGE(argc, 2, 2), -EINVAL, shell_warn(shell, "Invalid number of arguments!\n"));

	const size_t acSlots = AliroStack::Instance().GetConfig().mAccessCredentialKeySlots;

	constexpr char kAll[] = "all";
	if (strncmp(argv[1], kAll, CStrLen(kAll)) == 0) {
		shell_warn(shell, "Clearing all Access Credential public keys!\n");
		for (size_t keyId = 0; keyId < acSlots; keyId++) {
			int status = RemoveACKey(shell, keyId);
			VerifyOrReturnStatus(status == 0, status);
		}
		return 0;
	}

	size_t keyId{};
	int status = ParseKeyId(shell, argv[1], keyId);
	VerifyOrReturnStatus(status == 0, status);

	status = RemoveACKey(shell, keyId);
	VerifyOrReturnStatus(status == 0, status);
	return 0;
}

int FactoryReset(const struct shell *shell, size_t, char **)
{
	void *storage{ nullptr };
	int status = settings_storage_get(&storage);
	VerifyOrReturnStatus(status == 0, -EIO, shell_warn(shell, "Cannot get storage\n"));
	status = nvs_clear(static_cast<nvs_fs *>(storage));
	VerifyOrReturnStatus(status == 0, -EIO, shell_warn(shell, "Cannot clear storage\n"));
	k_work_reschedule(&sRebootWork, K_MSEC(250));
	return 0;
}

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

void RegisterShellCommands()
{
	k_work_init_delayable(&sRebootWork, [](k_work *) { sys_reboot(SYS_REBOOT_WARM); });

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
		SHELL_CMD(
			group_sub_id, NULL,
			"Set or get group sub ID\n"
			"  Usage: dl install group_sub_id <16-byte reader_group_sub_identifier in hex in hex without 0x>",
			ShellCmdHandleIdentifiers),
		SHELL_SUBCMD_SET_END);

	SHELL_STATIC_SUBCMD_SET_CREATE(
		AC_key_cmd,
		SHELL_CMD(list, NULL,
			  "List Access Credential public keys\n"
			  "  Usage: dl AC_key list",
			  ShellCmdHandleAccessCredentialList),
		SHELL_CMD(set, NULL,
			  "Set Access Credential public key\n"
			  "  Usage: dl AC_key set <key_id> <65-byte public key in hex in hex without 0x>",
			  ShellCmdHandleAccessCredentialSet),
		SHELL_CMD(clear, NULL,
			  "Clear Access Credential public key\n"
			  "  Usage: dl AC_key clear <key_id>\n"
			  "         dl AC_key clear all\n",
			  ShellCmdHandleAccessCredentialClear),
		SHELL_SUBCMD_SET_END);

	SHELL_STATIC_SUBCMD_SET_CREATE(provisioning_cmd,
				       SHELL_CMD(AC_key, &AC_key_cmd, "Manage Access Credential public keys", NULL),
				       SHELL_SUBCMD_SET_END);

	SHELL_STATIC_SUBCMD_SET_CREATE(door_lock_cmd, SHELL_CMD(install, &install_cmd, "Installation commands", NULL),
				       SHELL_CMD(provisioning, &provisioning_cmd, "Provisioning commands", NULL),
				       SHELL_CMD(factory_reset, NULL, "Factory reset", FactoryReset),
				       SHELL_CMD(info, NULL, "Show Aliro lib version and NFC reader chip name",
						 ShellCmdHandleInfo),
				       SHELL_SUBCMD_SET_END);

	SHELL_CMD_REGISTER(dl, &door_lock_cmd, "Door lock commands", NULL);
}
