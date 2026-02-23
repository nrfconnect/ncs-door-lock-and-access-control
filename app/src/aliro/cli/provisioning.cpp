/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "shell_private.h"

#include <aliro/aliro.h>
#include <aliro/utils.h>

#include "aliro/crypto_key_ids.h"
#include "aliro/utils/hex_string.h"
#include "crypto/utils.h"
#include "reader_cache.h"

#include <zephyr/shell/shell.h>

#include "../aliro_state_control.h"
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

#ifdef CONFIG_DOOR_LOCK_READER_CERTIFICATE
using CertificateData = std::array<uint8_t, StorageKeys::kMaxCertificateSize>;
#endif // CONFIG_DOOR_LOCK_READER_CERTIFICATE

using PublicKeyType = AccessManager::PublicKeyType;

constexpr size_t kAcMaxKeys{ CONFIG_DOOR_LOCK_ACCESS_MANAGER_ACCESS_CREDENTIAL_MAX_STORED_KEYS };
[[maybe_unused]] constexpr size_t kCiMaxKeys{ CONFIG_DOOR_LOCK_ACCESS_MANAGER_CREDENTIAL_ISSUER_MAX_STORED_KEYS };
constexpr size_t kPublicKeyStringLength{ 2 * CryptoTypes::kEccP256PublicKeyLength };
constexpr size_t kPrivateKeyStringLength{ 2 * CryptoTypes::kEccP256KeyPrivateKeyLength };

constexpr PublicKeyType GetPublicKeyTypeFromStorageKey(KeyValueStorage::KeyIdString keyName)
{
	return keyName == StorageKeys::kStorageKeyNameAccessCredentialPublicKey ? PublicKeyType::AccessCredential :
										  PublicKeyType::CredentialIssuer;
}

int ShellCmdHandleReaderPrivateKeySet(const struct shell *shell, size_t argc, char **argv)
{
	VerifyOrReturnStatus(argc == 2, -EINVAL, shell_warn(shell, "Invalid number of arguments!\n"));
	VerifyOrReturnValue(IsShellInitialized(), -EIO, shell_warn(shell, "Not initialized yet\n"));

	const char *keyStr{ argv[1] };
	size_t len = strlen(keyStr);
	VerifyOrReturnStatus(len == kPrivateKeyStringLength, -EINVAL,
			     shell_warn(shell, "Invalid key length (must be %zu hex chars)\n",
					kPrivateKeyStringLength));

	CryptoTypes::PrivateKey privateKey{};
	const size_t decodedLen = hex2bin(keyStr, len, privateKey.data(), privateKey.size());
	VerifyOrReturnStatus(decodedLen == privateKey.size(), -EINVAL, shell_warn(shell, "Invalid key hex string!\n"));

	CryptoTypes::KeyId keyId{ kPrivateKeyId };
	const auto err = DoorLock::Crypto::ImportPrivateKey(privateKey, true, keyId);
	VerifyOrReturnStatus(err == ALIRO_NO_ERROR, -EIO, shell_warn(shell, "Failed to import private key\n"));

	CryptoTypes::PublicKey publicKey{};
	const auto exportErr = DoorLock::Crypto::ExportPublicKey(keyId, publicKey);
	VerifyOrReturnStatus(exportErr == ALIRO_NO_ERROR, -EIO, shell_warn(shell, "Failed to export public key\n"));

	AliroError cacheErr = ReaderCache::Instance().SetPublicKey(publicKey);
	VerifyOrReturnStatus(cacheErr == ALIRO_NO_ERROR, -EIO, shell_warn(shell, "Failed to cache public key\n"));

	AliroError ec = DoorLock::AliroStateControl::UpdateAliroState();
	VerifyOrReturnValue(ec == ALIRO_NO_ERROR, -EIO,
			    shell_warn(shell, "Failed to update Aliro state: %d\n", ec.ToInt()));

	return 0;
}

int ShellCmdHandleReaderPrivateKeyClear(const struct shell *shell, size_t argc, char **)
{
	VerifyOrReturnStatus(argc == 1, -EINVAL, shell_warn(shell, "Invalid number of arguments!\n"));
	VerifyOrReturnValue(IsShellInitialized(), -EIO, shell_warn(shell, "Not initialized yet\n"));

	CryptoTypes::KeyId keyId{ kPrivateKeyId };
	const auto err = DoorLock::Crypto::DestroyKey(keyId);
	VerifyOrReturnStatus(err == ALIRO_NO_ERROR, -EIO, shell_warn(shell, "Failed to destroy private key\n"));

	ReaderCache::Instance().ClearPublicKey();

	shell_print(shell, "Reader private key cleared");

	AliroError ec = DoorLock::AliroStateControl::UpdateAliroState();
	VerifyOrReturnValue(ec == ALIRO_NO_ERROR, -EIO,
			    shell_warn(shell, "Failed to update Aliro state: %d\n", ec.ToInt()));

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
	const auto error =
		DoorLock::Crypto::ExportKey(kCredentialIssuerCAPublicKeyId, publicKey.data(), publicKey.size());
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

	CryptoTypes::KeyId keyId{ kCredentialIssuerCAPublicKeyId };
	const auto error = DoorLock::Crypto::ImportPublicKey(publicKey, true, keyId);
	VerifyOrReturnStatus(error == ALIRO_NO_ERROR, -EINVAL,
			     shell_warn(shell, "Cannot import Credential Issuer CA public key\n"));

	return 0;
}

int ShellCmdHandleCredentialIssuerCAClear(const struct shell *shell, size_t argc, char **argv)
{
	VerifyOrReturnStatus(argc == 1, -EINVAL, shell_warn(shell, "Invalid number of arguments!\n"));
	VerifyOrReturnValue(IsShellInitialized(), -EIO, shell_warn(shell, "Not initialized yet\n"));

	CryptoTypes::KeyId keyId{ kCredentialIssuerCAPublicKeyId };
	const auto error = DoorLock::Crypto::DestroyKey(keyId);
	VerifyOrReturnStatus(error == ALIRO_NO_ERROR, -EINVAL,
			     shell_warn(shell, "Cannot remove Credential Issuer CA public key\n"));

	return 0;
}

#endif // CONFIG_DOOR_LOCK_CREDENTIAL_ISSUER_CA

#ifdef CONFIG_DOOR_LOCK_READER_CERTIFICATE

int ShellCmdHandleReaderCertList(const struct shell *shell, size_t, char **)
{
	VerifyOrReturnValue(IsShellInitialized(), -EIO, shell_warn(shell, "Not initialized yet\n"));

	ConstData cert{};
	const auto error = ReaderCache::Instance().GetCertificate(cert);
	if (error == ALIRO_NO_ERROR) {
		DoorLock::Utils::HexStringBuffer<CertificateData> hexString{};
		CertificateData certificateData{};
		std::copy_n(cert.mData, cert.mLength, certificateData.begin());
		VerifyOrReturnStatus(DoorLock::Utils::ArrayToHexString(hexString, certificateData, cert.mLength),
				     -EINVAL, shell_warn(shell, "Invalid certificate hex string!\n"));
		shell_print(shell, "Reader certificate (%zu bytes): %s", cert.mLength, hexString.data());
	} else {
		shell_print(shell, "Reader certificate: (not set)");
	}

	return 0;
}

int ShellCmdHandleReaderCertSet(const struct shell *shell, size_t argc, char **argv)
{
	VerifyOrReturnStatus(IN_RANGE(argc, 2, 2), -EINVAL, shell_warn(shell, "Invalid number of arguments!\n"));
	VerifyOrReturnValue(IsShellInitialized(), -EIO, shell_warn(shell, "Not initialized yet\n"));

	const char *certStr = argv[1];
	size_t certStrLen = strlen(certStr);
	VerifyOrReturnStatus(certStrLen > 0 && certStrLen % 2 == 0, -EINVAL,
			     shell_warn(shell, "Invalid certificate length (must be even)!\n"));

	CertificateData certData{};
	size_t decodedLen = hex2bin(certStr, certStrLen, certData.data(), certData.size());
	VerifyOrReturnStatus(decodedLen == certStrLen / 2, -EINVAL,
			     shell_warn(shell, "Invalid certificate hex string!\n"));

	AliroError error = ReaderCache::Instance().SetCertificate({ certData.data(), decodedLen });
	VerifyOrReturnStatus(error == ALIRO_NO_ERROR, -EINVAL,
			     shell_warn(shell, "Failed to set certificate: %d\n", error.ToInt()));

	VerifyOrReturnStatus(!KeyValueStorage::Instance().Save(StorageKeys::kStorageKeyNameReaderCertificate,
							       certData.data(), decodedLen),
			     -EINVAL, shell_warn(shell, "Cannot save certificate to persistent storage!\n"));

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
	VerifyOrReturnValue(IsShellInitialized(), -EIO, shell_warn(shell, "Not initialized yet\n"));

	ReaderCache::Instance().ClearCertificate();

	VerifyOrReturnStatus(!KeyValueStorage::Instance().Clear(StorageKeys::kStorageKeyNameReaderCertificate), -EIO,
			     shell_warn(shell, "Cannot clear certificate from persistent storage!\n"));

	VerifyOrReturnStatus(!KeyValueStorage::Instance().Clear(StorageKeys::kStorageKeyNameReaderCertificateLength),
			     -EIO, shell_warn(shell, "Cannot clear certificate length from persistent storage!\n"));

	shell_print(shell, "Reader certificate cleared successfully");
	return 0;
}

int ShellCmdHandleIssuerPublicKeyList(const struct shell *shell, size_t, char **)
{
	VerifyOrReturnValue(IsShellInitialized(), -EIO, shell_warn(shell, "Shell not initialized\n"));

	CryptoTypes::PublicKey publicKey{};
	const auto error = ReaderCache::Instance().GetIssuerPublicKey(publicKey);
	if (error == ALIRO_NO_ERROR) {
		DoorLock::Utils::HexStringBuffer<CryptoTypes::PublicKey> hexString{};
		VerifyOrReturnStatus(DoorLock::Utils::ArrayToHexString(hexString, publicKey), -EINVAL,
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
	VerifyOrReturnValue(IsShellInitialized(), -EIO, shell_warn(shell, "Shell not initialized\n"));

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

	VerifyOrReturnStatus(
		!KeyValueStorage::Instance().Save(StorageKeys::kStorageKeyNameReaderSystemIssuerCAPublicKey,
						  publicKey.data(), publicKey.size()),
		-EINVAL, shell_warn(shell, "Cannot save issuer public key to persistent storage!\n"));

	VerifyOrReturnStatus(ReaderCache::Instance().SetIssuerPublicKey(publicKey) == ALIRO_NO_ERROR, -EINVAL,
			     shell_warn(shell, "Failed to set issuer public key!\n"));

	shell_print(shell, "Issuer public key set successfully (%zu bytes)", publicKey.size());

	return 0;
}

int ShellCmdHandleIssuerPublicKeyClear(const struct shell *shell, size_t argc, char **)
{
	VerifyOrReturnStatus(IN_RANGE(argc, 1, 1), -EINVAL, shell_warn(shell, "Invalid number of arguments!\n"));
	VerifyOrReturnValue(IsShellInitialized(), -EIO, shell_warn(shell, "Shell not initialized\n"));

	VerifyOrReturnStatus(
		!KeyValueStorage::Instance().Clear(StorageKeys::kStorageKeyNameReaderSystemIssuerCAPublicKey), -EIO,
		shell_warn(shell, "Cannot clear issuer public key from persistent storage!\n"));

	ReaderCache::Instance().ClearIssuerPublicKey();

	shell_print(shell, "Issuer public key cleared successfully");
	return 0;
}

#endif // CONFIG_DOOR_LOCK_READER_CERTIFICATE

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

SHELL_STATIC_SUBCMD_SET_CREATE(reader_prv_cmd,
			       SHELL_CMD(set, NULL,
					 "Set Reader private signing key (32 bytes)\n"
					 "  Usage: dl provisioning reader_prv set <64-hex-chars>",
					 ShellCmdHandleReaderPrivateKeySet),
			       SHELL_CMD(clear, NULL,
					 "Clear Reader private signing key\n"
					 "  Usage: dl provisioning reader_prv clear",
					 ShellCmdHandleReaderPrivateKeyClear),
			       SHELL_SUBCMD_SET_END);

SHELL_STATIC_SUBCMD_SET_CREATE(provisioning_cmd,
			       SHELL_CMD(reader_prv, &reader_prv_cmd, "Manage Reader private signing key", NULL),
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

} // namespace

SHELL_SUBCMD_ADD((dl), provisioning, &provisioning_cmd, "Provisioning commands", NULL, 0, 0);
