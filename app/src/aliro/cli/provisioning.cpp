/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "shell_private.h"

#include <aliro/aliro.h>
#include <aliro/utils.h>

#include "aliro/utils/hex_string.h"
#include "crypto/utils.h"
#include "psa_key_ids.h"

#include <zephyr/shell/shell.h>

#include "access_manager.h"
#include "storage.h"
#include "storage_keys.h"

#ifdef CONFIG_DOOR_LOCK_STEP_UP_PHASE
#include "validity_iterations.h"
#endif // CONFIG_DOOR_LOCK_STEP_UP_PHASE

#include <zephyr/sys/util.h>

#include <algorithm>
#include <cstdlib>

namespace {
using namespace Aliro;

using PublicKeyType = AccessManager::PublicKeyType;

constexpr size_t kAcMaxKeys{ CONFIG_DOOR_LOCK_ACCESS_MANAGER_ACCESS_CREDENTIAL_MAX_STORED_KEYS };
[[maybe_unused]] constexpr size_t kCiMaxKeys{ CONFIG_DOOR_LOCK_ACCESS_MANAGER_CREDENTIAL_ISSUER_MAX_STORED_KEYS };
constexpr size_t kPublicKeyStringLength{ 2 * CryptoTypes::kEccP256PublicKeyLength };

constexpr PublicKeyType GetPublicKeyTypeFromStorageKey(KeyValueStorage::KeyIdString keyName)
{
	return keyName == StorageKeys::kStorageKeyNameAccessCredentialPublicKey ? PublicKeyType::AccessCredential :
										  PublicKeyType::CredentialIssuer;
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
	VerifyOrReturnValue(IsShellInitialized(), -EIO, shell_warn(shell, "Not initialized yet\n"));

	DoorLock::Utils::HexStringBuffer<CryptoTypes::PublicKey> hexString{};
	CryptoTypes::PublicKey publicKey{};

	for (size_t keyId = 0; keyId < numSlots; keyId++) {
		if (GetPublicKeyFromStorage(keyIdStr, keyId, publicKey) == 0) {
			if (!DoorLock::Utils::ArrayToHexString(hexString, publicKey)) {
				snprintf(hexString.data(), hexString.size(), "(invalid)");
			}
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
	VerifyOrReturnValue(IsShellInitialized(), -EIO, shell_warn(shell, "Not initialized yet\n"));

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
	VerifyOrReturnValue(IsShellInitialized(), -EIO, shell_warn(shell, "Not initialized yet\n"));

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
	const int rc = CmdHandleAndSetKey(shell, argc, argv, StorageKeys::kStorageKeyNameAccessCredentialPublicKey,
					  kAcMaxKeys, keyId);
	VerifyOrReturnValue(rc == 0, rc);

	return 0;
}

int ShellCmdHandleAccessCredentialClear(const struct shell *shell, size_t argc, char **argv)
{
	size_t keyId{};
	const int rc = CmdHandleAndClearKey(shell, argc, argv, StorageKeys::kStorageKeyNameAccessCredentialPublicKey,
					    kAcMaxKeys, keyId);
	VerifyOrReturnValue(rc == 0, rc);

	return 0;
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
	VerifyOrReturnValue(IsShellInitialized(), -EIO, shell_warn(shell, "Not initialized yet\n"));

	CryptoTypes::PublicKey publicKey{};
	const auto error = DoorLock::Crypto::ExportKey(DoorLock::Storage::PsaKeyIds::kCredentialIssuerCAPublicKeyId,
						       publicKey.data(), publicKey.size());
	VerifyOrReturnStatus(error == ALIRO_NO_ERROR, -EINVAL,
			     shell_warn(shell, "Cannot export Credential Issuer CA public key\n"));

	DoorLock::Utils::HexStringBuffer<CryptoTypes::PublicKey> hexString{};
	VerifyOrReturnStatus(DoorLock::Utils::ArrayToHexString(hexString, publicKey), -EINVAL,
			     shell_warn(shell, "Cannot convert Credential Issuer CA public key to hex\n"));
	shell_print(shell, "%s", hexString.data());

	return 0;
}

int ShellCmdHandleCredentialIssuerCASet(const struct shell *shell, size_t argc, char **argv)
{
	VerifyOrReturnStatus(argc == 2, -EINVAL, shell_warn(shell, "Invalid number of arguments!\n"));
	VerifyOrReturnValue(IsShellInitialized(), -EIO, shell_warn(shell, "Not initialized yet\n"));

	const char *pubkeyStr{ argv[1] };
	size_t len = strlen(pubkeyStr);
	VerifyOrReturnStatus(len == kPublicKeyStringLength, -EINVAL, shell_warn(shell, "Invalid key length!\n"));

	CryptoTypes::PublicKey publicKey{};
	const auto result = hex2bin(pubkeyStr, len, publicKey.data(), publicKey.size());
	VerifyOrReturnStatus(result == publicKey.size(), -EINVAL, shell_warn(shell, "Invalid key length!\n"));

	VerifyOrReturnStatus(publicKey[0] == CryptoTypes::kEccP256PublicKeyPrefix, -EINVAL,
			     shell_warn(shell, "Invalid key prefix!\n"));

	CryptoTypes::KeyId keyId{ DoorLock::Storage::PsaKeyIds::kCredentialIssuerCAPublicKeyId };
	const auto error = DoorLock::Crypto::ImportPublicKey(publicKey, true, keyId);
	VerifyOrReturnStatus(error == ALIRO_NO_ERROR, -EINVAL,
			     shell_warn(shell, "Cannot import Credential Issuer CA public key\n"));

	return 0;
}

int ShellCmdHandleCredentialIssuerCAClear(const struct shell *shell, size_t argc, char **argv)
{
	VerifyOrReturnStatus(argc == 1, -EINVAL, shell_warn(shell, "Invalid number of arguments!\n"));
	VerifyOrReturnValue(IsShellInitialized(), -EIO, shell_warn(shell, "Not initialized yet\n"));

	CryptoTypes::KeyId keyId{ DoorLock::Storage::PsaKeyIds::kCredentialIssuerCAPublicKeyId };
	const auto error = DoorLock::Crypto::DestroyKey(keyId);
	VerifyOrReturnStatus(error == ALIRO_NO_ERROR, -EINVAL,
			     shell_warn(shell, "Cannot remove Credential Issuer CA public key\n"));

	return 0;
}

#endif // CONFIG_DOOR_LOCK_CREDENTIAL_ISSUER_CA

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

SHELL_STATIC_SUBCMD_SET_CREATE(provisioning_cmd,
			       SHELL_CMD(AC_key, &AC_key_cmd, "Manage Access Credential public keys", NULL),

#if CONFIG_DOOR_LOCK_ACCESS_MANAGER_CREDENTIAL_ISSUER_MAX_STORED_KEYS > 0
			       SHELL_CMD(CI_key, &CI_key_cmd, "Manage Credential Issuer public keys", NULL),
#endif // CONFIG_DOOR_LOCK_ACCESS_MANAGER_CREDENTIAL_ISSUER_MAX_STORED_KEYS > 0

#ifdef CONFIG_DOOR_LOCK_CREDENTIAL_ISSUER_CA
			       SHELL_CMD(CI_CA_key, &CI_CA_key_cmd, "Manage Credential Issuer CA public key", NULL),
#endif // CONFIG_DOOR_LOCK_CREDENTIAL_ISSUER_CA
			       SHELL_SUBCMD_SET_END);

} // namespace

SHELL_SUBCMD_ADD((dl), provisioning, &provisioning_cmd, "Provisioning commands", NULL, 0, 0);
