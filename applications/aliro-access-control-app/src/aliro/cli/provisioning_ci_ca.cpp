/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "shell_private.h"

#include <aliro/aliro.h>
#include <aliro/utils.h>
#include <crypto_utils/crypto_utils.h>

#include "aliro/utils/hex_string.h"
#include "psa_key_ids.h"

#include <zephyr/shell/shell.h>

namespace {
using namespace Aliro;

constexpr size_t kPublicKeyStringLength{ 2 * CryptoTypes::kEccP256PublicKeyLength };

int ShellCmdHandleCredentialIssuerCAList(const struct shell *shell, size_t argc, char **)
{
	VerifyOrReturnStatus(argc == 1, -EINVAL, shell_warn(shell, "Invalid number of arguments!\n"));
	VerifyOrReturnValue(IsShellInitialized(), -EIO, shell_warn(shell, "Not initialized yet\n"));

	Aliro::CryptoTypes::KeyId keyId{ DoorLock::Storage::PsaKeyIds::kCredentialIssuerCAPublicKeyId };
	const auto keyAvailable = DoorLock::CryptoUtils::IsKeyAvailable(keyId) == ALIRO_NO_ERROR;
	if (!keyAvailable) {
		shell_warn(shell, "Credential Issuer CA public key is not set\n");
		return 0;
	}

	CryptoTypes::PublicKey publicKey{};
	const auto error = DoorLock::CryptoUtils::ExportKey(keyId, publicKey.data(), publicKey.size());
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
	const auto keyAvailable = DoorLock::CryptoUtils::IsKeyAvailable(keyId) == ALIRO_NO_ERROR;
	if (keyAvailable) {
		const auto error = DoorLock::CryptoUtils::DestroyKey(keyId);
		VerifyOrReturnStatus(error == ALIRO_NO_ERROR, -EINVAL,
				     shell_warn(shell, "Cannot remove Credential Issuer CA public key\n"));
	}

	keyId = DoorLock::Storage::PsaKeyIds::kCredentialIssuerCAPublicKeyId;
	const auto error = DoorLock::CryptoUtils::ImportPublicKey(publicKey, true, keyId);
	VerifyOrReturnStatus(error == ALIRO_NO_ERROR, -EINVAL,
			     shell_warn(shell, "Cannot import Credential Issuer CA public key\n"));

	return 0;
}

int ShellCmdHandleCredentialIssuerCAClear(const struct shell *shell, size_t argc, char **argv)
{
	VerifyOrReturnStatus(argc == 1, -EINVAL, shell_warn(shell, "Invalid number of arguments!\n"));
	VerifyOrReturnValue(IsShellInitialized(), -EIO, shell_warn(shell, "Not initialized yet\n"));

	CryptoTypes::KeyId keyId{ DoorLock::Storage::PsaKeyIds::kCredentialIssuerCAPublicKeyId };
	const auto error = DoorLock::CryptoUtils::DestroyKey(keyId);
	VerifyOrReturnStatus(error == ALIRO_NO_ERROR, -EINVAL,
			     shell_warn(shell, "Cannot remove Credential Issuer CA public key\n"));

	return 0;
}

SHELL_STATIC_SUBCMD_SET_CREATE(CI_CA_key_cmd,
			       SHELL_CMD(list, NULL,
					 "List Credential Issuer CA public key\n"
					 "  Usage: dl CI_CA_key list",
					 ShellCmdHandleCredentialIssuerCAList),
			       SHELL_CMD(set, NULL,
					 "Set Credential Issuer CA public key\n"
					 "  Usage: dl CI_CA_key set <65-byte public key in hex without 0x>",
					 ShellCmdHandleCredentialIssuerCASet),
			       SHELL_CMD(clear, NULL,
					 "Clear Credential Issuer CA public key\n"
					 "  Usage: dl CI_CA_key clear\n",
					 ShellCmdHandleCredentialIssuerCAClear),
			       SHELL_SUBCMD_SET_END);
SHELL_SUBCMD_ADD((provisioning), CI_CA_key, &CI_CA_key_cmd, "Manage Credential Issuer CA public key", NULL, 0, 0);

} // namespace
