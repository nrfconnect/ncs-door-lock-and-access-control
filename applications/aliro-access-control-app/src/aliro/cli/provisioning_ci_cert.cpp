/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "shell_private.h"

#include <aliro/aliro.h>
#include <doorlock/utils/cstr.h>
#include <doorlock/utils/utils.h>

#include "aliro/utils/hex_string.h"

#include <zephyr/shell/shell.h>

#include "access_manager.h"
#include "credential_issuer_keys.h"

#include <cstdlib>

namespace {
using namespace Aliro;

using PublicKeyType = AccessManager::PublicKeyType;

constexpr size_t kCiCertMaxKeys{ CONFIG_DOOR_LOCK_ACCESS_MANAGER_CREDENTIAL_ISSUER_CERTIFICATE_MAX_STORED_KEYS };

int ParseKeyId(const struct shell *shell, const char *keyIdStr, size_t &keyId)
{
	char *endPtr{ nullptr };
	errno = 0;
	const unsigned long val = strtoul(keyIdStr, &endPtr, 10);
	VerifyOrReturnValue(errno == 0 && endPtr != keyIdStr, -EINVAL, shell_warn(shell, "Invalid key ID!\n"));

	VerifyOrReturnValue(kCiCertMaxKeys > 0, -EINVAL,
			    shell_warn(shell, "Certificate Credential Issuer public key slots not supported!\n"));

	VerifyOrReturnValue(IN_RANGE(val, 0, kCiCertMaxKeys - 1), -EINVAL,
			    shell_warn(shell, "Key ID out of range, must be between 0 and %u!\n", kCiCertMaxKeys - 1));

	keyId = val;
	return 0;
}

int RemoveCertificateCredentialIssuerKey(const struct shell *shell, size_t keyId)
{
	CryptoTypes::PublicKey publicKey{};
	const auto readError = ReadCertificateCredentialIssuerKey(keyId, publicKey);
	VerifyOrReturnValue(readError != ALIRO_PUBLIC_KEY_NOT_FOUND, -EINVAL,
			    shell_warn(shell, "Certificate Credential Issuer public key at index %u does not exist\n",
				       keyId));
	VerifyOrReturnValue(readError == ALIRO_NO_ERROR, -EINVAL,
			    shell_warn(shell, "Cannot read Certificate Credential Issuer public key\n"));

	VerifyOrReturnValue(
		AccessManagerInstance().RemovePublicKey(PublicKeyType::CertificateCredentialIssuer, keyId) ==
			ALIRO_NO_ERROR,
		-EINVAL,
		shell_warn(shell, "Cannot remove Certificate Credential Issuer public key from Access Manager\n"));

	return 0;
}

int ShellCmdHandleCertificateCredentialIssuerList(const struct shell *shell, size_t argc, char **)
{
	VerifyOrReturnValue(argc == 1, -EINVAL, shell_warn(shell, "Invalid number of arguments!\n"));
	VerifyOrReturnValue(IsShellInitialized(), -EIO, shell_warn(shell, "Not initialized yet\n"));

	DoorLock::Utils::HexStringBuffer<CryptoTypes::PublicKey> hexString{};
	CryptoTypes::PublicKey publicKey{};

	for (size_t keyId = 0; keyId < kCiCertMaxKeys; keyId++) {
		const auto error = ReadCertificateCredentialIssuerKey(keyId, publicKey);
		if (error == ALIRO_PUBLIC_KEY_NOT_FOUND) {
			snprintf(hexString.data(), hexString.size(), "(null)");
		} else {
			VerifyOrReturnValue(error == ALIRO_NO_ERROR, -EINVAL,
					    shell_warn(shell,
						       "Cannot read Certificate Credential Issuer public key\n"));

			if (!DoorLock::Utils::ArrayToHexString(hexString, publicKey)) {
				snprintf(hexString.data(), hexString.size(), "(invalid)");
			}
		}

		shell_print(shell, "[%u]: %s", keyId, hexString.data());
	}

	return 0;
}

int ShellCmdHandleCertificateCredentialIssuerClear(const struct shell *shell, size_t argc, char **argv)
{
	VerifyOrReturnValue(argc == 2, -EINVAL, shell_warn(shell, "Invalid number of arguments!\n"));
	VerifyOrReturnValue(IsShellInitialized(), -EIO, shell_warn(shell, "Not initialized yet\n"));

	constexpr char kAll[] = "all";
	if (strncmp(argv[1], kAll, DoorLock::Utils::CStrSize(kAll)) == 0) {
		shell_warn(shell, "Clearing all Certificate Credential Issuer public keys!\n");
		for (size_t keyId = 0; keyId < kCiCertMaxKeys; keyId++) {
			CryptoTypes::PublicKey publicKey{};
			const auto readError = ReadCertificateCredentialIssuerKey(keyId, publicKey);
			if (readError == ALIRO_PUBLIC_KEY_NOT_FOUND) {
				continue;
			}
			VerifyOrReturnValue(readError == ALIRO_NO_ERROR, -EINVAL,
					    shell_warn(shell,
						       "Cannot read Certificate Credential Issuer public key\n"));

			const int status = RemoveCertificateCredentialIssuerKey(shell, keyId);
			VerifyOrReturnValue(status == 0, status);
		}
		return 0;
	}

	size_t keyId{};
	const int status = ParseKeyId(shell, argv[1], keyId);
	VerifyOrReturnValue(status == 0, status);

	return RemoveCertificateCredentialIssuerKey(shell, keyId);
}

SHELL_STATIC_SUBCMD_SET_CREATE(CI_cert_key_cmd,
			       SHELL_CMD(list, NULL,
					 "List Certificate Credential Issuer public keys learned from certificates\n"
					 "  Usage: dl CI_cert_key list",
					 ShellCmdHandleCertificateCredentialIssuerList),
			       SHELL_CMD(clear, NULL,
					 "Clear Certificate Credential Issuer public key learned from a certificate\n"
					 "  Usage: dl CI_cert_key clear <key_id>\n"
					 "         dl CI_cert_key clear all\n",
					 ShellCmdHandleCertificateCredentialIssuerClear),
			       SHELL_SUBCMD_SET_END);
SHELL_SUBCMD_ADD((provisioning), CI_cert_key, &CI_cert_key_cmd,
		 "Manage Certificate Credential Issuer public keys learned from certificates", NULL, 0, 0);

} // namespace
