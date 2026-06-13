/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <reader_storage/reader.h>

#include "cert.h"
#include "shell.h"

#include <aliro/utils.h>
#include <doorlock/utils/cstr.h>

#include <zephyr/shell/shell.h>
#include <zephyr/sys/util.h>

#include <array>
#include <cstring>

namespace DoorLock::ReaderStorage {

namespace {

using namespace Aliro;

constexpr size_t kMinimumShellCmdBufferSize{ DoorLock::Utils::CStrLen("reader certificate set ") +
					     2 * kMaxCertificateSize };

static_assert(CONFIG_SHELL_CMD_BUFF_SIZE >= kMinimumShellCmdBufferSize, "Shell command buffer size is too small");

int HandleCertificateList(const struct shell *shell, size_t, char **)
{
	VerifyOrReturnValue(IsInitialized(), -EIO, shell_warn(shell, "Reader storage not initialized\n"));

	if (!ReaderStorage::IsCertificateSet()) {
		shell_warn(shell, "Reader certificate not set\n");
		return 0;
	}

	ConstData certData{};
	const auto error = ReaderStorage::GetCertificate(certData);
	VerifyOrReturnValue(error == ALIRO_NO_ERROR, -EIO,
			    shell_warn(shell, "Failed to get Reader certificate: %d\n", error.ToInt()));
	VerifyOrReturnValue(certData.mData && IN_RANGE(certData.mLength, 1, kMaxCertificateSize), -EINVAL,
			    shell_warn(shell, "Invalid Reader certificate data\n"));

	shell_print(shell, "Reader certificate (%zu bytes)", certData.mLength);
	shell_hexdump(shell, certData.mData, certData.mLength);

	return 0;
}

int HandleCertificateSet(const struct shell *shell, size_t argc, char **argv)
{
	VerifyOrReturnValue(IsInitialized(), -EIO, shell_warn(shell, "Reader storage not initialized\n"));

	VerifyOrReturnValue(argc == 2, -EINVAL, shell_warn(shell, "Usage: reader certificate set <hex-certificate>\n"));

	const char *certificateString = argv[1];
	const size_t certificateStringLength = strlen(certificateString);
	VerifyOrReturnValue(certificateStringLength > 0 && certificateStringLength % 2 == 0, -EINVAL,
			    shell_warn(shell, "Invalid Reader certificate length (must be even)\n"));

	std::array<uint8_t, kMaxCertificateSize> certificate{};
	const size_t certificateLength =
		hex2bin(certificateString, certificateStringLength, certificate.data(), certificate.size());
	VerifyOrReturnValue(certificateLength == certificateStringLength / 2, -EINVAL,
			    shell_warn(shell, "Invalid Reader certificate hex string!\n"));

	const auto error = ReaderStorage::SetCertificate(certificate.data(), certificateLength);
	VerifyOrReturnValue(error == ALIRO_NO_ERROR, -EIO,
			    shell_warn(shell, "Failed to set Reader certificate: %d\n", error.ToInt()));

	NotifyDataChanged();
	return 0;
}

int HandleCertificateClear(const struct shell *shell, size_t, char **)
{
	VerifyOrReturnValue(IsInitialized(), -EIO, shell_warn(shell, "Reader storage not initialized\n"));

	const auto error = ReaderStorage::ClearCertificate();
	VerifyOrReturnValue(error == ALIRO_NO_ERROR, -EIO,
			    shell_warn(shell, "Failed to clear Reader certificate: %d\n", error.ToInt()));

	NotifyDataChanged();
	return 0;
}

int HandleIssuerPublicKeyList(const struct shell *shell, size_t, char **)
{
	VerifyOrReturnValue(IsInitialized(), -EIO, shell_warn(shell, "Reader storage not initialized\n"));

	if (!ReaderStorage::IsIssuerPublicKeySet()) {
		shell_warn(shell, "Reader System Issuer CA public key not set");
		return 0;
	}

	CryptoTypes::PublicKey publicKey{};
	const auto error = ReaderStorage::GetIssuerPublicKey(publicKey);
	VerifyOrReturnValue(error == ALIRO_NO_ERROR, -EIO,
			    shell_warn(shell, "Failed to get Reader System Issuer CA public key: %d\n", error.ToInt()));

	char hexString[CryptoTypes::kEccP256PublicKeyLength * 2 + 1];
	const size_t convertedLength = bin2hex(publicKey.data(), publicKey.size(), hexString, std::size(hexString));
	VerifyOrReturnValue(convertedLength == publicKey.size() * 2, -EINVAL,
			    shell_warn(shell, "Reader System Issuer CA public key hex string invalid!\n"));

	shell_print(shell, "Reader System Issuer CA public key: %s", hexString);
	return 0;
}

int HandleIssuerPublicKeySet(const struct shell *shell, size_t argc, char **argv)
{
	VerifyOrReturnValue(IsInitialized(), -EIO, shell_warn(shell, "Reader storage not initialized\n"));

	VerifyOrReturnValue(argc == 2, -EINVAL,
			    shell_warn(shell, "Usage: reader issuer_public_key set <65-byte-hex-public-key>\n"));

	const char *keyString = argv[1];
	const size_t keyStringLength = strlen(keyString);
	VerifyOrReturnValue(keyStringLength == CryptoTypes::kEccP256PublicKeyLength * 2, -EINVAL,
			    shell_warn(shell, "Invalid key length (must be %zu hex characters)\n",
				       CryptoTypes::kEccP256PublicKeyLength * 2));

	CryptoTypes::PublicKey publicKey{};
	const size_t decoded = hex2bin(keyString, keyStringLength, publicKey.data(), publicKey.size());
	VerifyOrReturnValue(decoded == publicKey.size(), -EINVAL,
			    shell_warn(shell, "Invalid Reader System Issuer CA public key hex string!\n"));
	VerifyOrReturnValue(publicKey[0] == CryptoTypes::kEccP256PublicKeyPrefix, -EINVAL,
			    shell_warn(shell, "Invalid Reader System Issuer CA public key prefix (must be 0x04)\n"));

	const auto error = ReaderStorage::SetIssuerPublicKey(publicKey);
	VerifyOrReturnValue(error == ALIRO_NO_ERROR, -EIO,
			    shell_warn(shell, "Failed to set Reader System Issuer CA public key: %d\n", error.ToInt()));

	NotifyDataChanged();
	return 0;
}

int HandleIssuerPublicKeyClear(const struct shell *shell, size_t, char **)
{
	VerifyOrReturnValue(IsInitialized(), -EIO, shell_warn(shell, "Reader storage not initialized\n"));

	const auto error = ReaderStorage::ClearIssuerPublicKey();
	VerifyOrReturnValue(error == ALIRO_NO_ERROR, -EIO,
			    shell_warn(shell, "Failed to clear Reader System Issuer CA public key: %d\n",
				       error.ToInt()));

	NotifyDataChanged();
	return 0;
}

} // namespace

SHELL_STATIC_SUBCMD_SET_CREATE(certificate_cmd, SHELL_CMD(list, NULL, "List Reader certificate", HandleCertificateList),
			       SHELL_CMD(set, NULL, "Set Reader certificate", HandleCertificateSet),
			       SHELL_CMD(clear, NULL, "Clear Reader certificate", HandleCertificateClear),
			       SHELL_SUBCMD_SET_END);
SHELL_SUBCMD_ADD((reader), certificate, &certificate_cmd, "Reader certificate commands", NULL, 0, 0);

SHELL_STATIC_SUBCMD_SET_CREATE(issuer_public_key_cmd,
			       SHELL_CMD(list, NULL, "List Issuer public key", HandleIssuerPublicKeyList),
			       SHELL_CMD(set, NULL, "Set Issuer public key", HandleIssuerPublicKeySet),
			       SHELL_CMD(clear, NULL, "Clear Issuer public key", HandleIssuerPublicKeyClear),
			       SHELL_SUBCMD_SET_END);
SHELL_SUBCMD_ADD((reader), issuer_public_key, &issuer_public_key_cmd, "Reader System Issuer CA public key commands",
		 NULL, 0, 0);

} // namespace DoorLock::ReaderStorage
