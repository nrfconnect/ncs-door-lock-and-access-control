/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "shell_private.h"

#include "reader.h"

#include "aliro/utils.h"
#include "aliro/utils/hex_string.h"
#include "storage_keys.h"

#include <algorithm>
#include <array>
#include <zephyr/shell/shell.h>
#include <zephyr/sys/util.h>

namespace DoorLock::ReaderShell {
namespace {

using namespace Aliro;

int HandleCertificateList(const struct shell *shell, size_t, char **)
{
	VerifyOrReturnValue(IsShellInitialized(), -EIO, shell_warn(shell, "Shell not initialized\n"));

	if (!Storage::Reader::IsCertificateSet()) {
		shell_warn(shell, "Reader certificate not set");
		return 0;
	}

	ConstData certData{};
	const auto error = Storage::Reader::GetCertificate(certData);
	VerifyOrReturnValue(error == ALIRO_NO_ERROR, -EIO,
			    shell_warn(shell, "Failed to get Reader certificate: %d\n", error.ToInt()));
	VerifyOrReturnValue(certData.mData && IN_RANGE(certData.mLength, 1, StorageKeys::kMaxCertificateSize), -EINVAL,
			    shell_warn(shell, "Invalid Reader certificate data\n"));

	std::array<uint8_t, StorageKeys::kMaxCertificateSize> certificate{};
	std::copy_n(certData.mData, certData.mLength, certificate.begin());

	DoorLock::Utils::HexStringBuffer<decltype(certificate)> hexString{};
	VerifyOrReturnValue(DoorLock::Utils::ArrayToHexString(hexString, certificate, certData.mLength), -EINVAL,
			    shell_warn(shell, "Invalid Reader certificate hex string!\n"));
	shell_print(shell, "Reader certificate (%zu bytes): %s", certData.mLength, hexString.data());
	return 0;
}

int HandleCertificateSet(const struct shell *shell, size_t argc, char **argv)
{
	VerifyOrReturnValue(IsShellInitialized(), -EIO, shell_warn(shell, "Shell not initialized\n"));

	VerifyOrReturnValue(argc == 2, -EINVAL, shell_warn(shell, "Usage: reader certificate set <hex-certificate>\n"));

	const char *certificateString = argv[1];
	const size_t certificateStringLength = strlen(certificateString);
	VerifyOrReturnValue(certificateStringLength > 0 && certificateStringLength % 2 == 0, -EINVAL,
			    shell_warn(shell, "Invalid Reader certificate length (must be even)\n"));

	std::array<uint8_t, StorageKeys::kMaxCertificateSize> certificate{};
	const size_t certificateLength =
		hex2bin(certificateString, certificateStringLength, certificate.data(), certificate.size());
	VerifyOrReturnValue(certificateLength == certificateStringLength / 2, -EINVAL,
			    shell_warn(shell, "Invalid Reader certificate hex string!\n"));

	const auto error = Storage::Reader::SetCertificate(certificate.data(), certificateLength);
	VerifyOrReturnValue(error == ALIRO_NO_ERROR, -EIO,
			    shell_warn(shell, "Failed to set Reader certificate: %d\n", error.ToInt()));
	return 0;
}

int HandleCertificateClear(const struct shell *shell, size_t, char **)
{
	VerifyOrReturnValue(IsShellInitialized(), -EIO, shell_warn(shell, "Shell not initialized\n"));

	const auto error = Storage::Reader::ClearCertificate();
	VerifyOrReturnValue(error == ALIRO_NO_ERROR, -EIO,
			    shell_warn(shell, "Failed to clear Reader certificate: %d\n", error.ToInt()));
	return 0;
}

int HandleIssuerPublicKeyList(const struct shell *shell, size_t, char **)
{
	VerifyOrReturnValue(IsShellInitialized(), -EIO, shell_warn(shell, "Shell not initialized\n"));

	if (!Storage::Reader::IsIssuerPublicKeySet()) {
		shell_warn(shell, "Reader System Issuer CA public key not set");
		return 0;
	}

	CryptoTypes::PublicKey publicKey{};
	const auto error = Storage::Reader::GetIssuerPublicKey(publicKey);
	VerifyOrReturnValue(error == ALIRO_NO_ERROR, -EIO,
			    shell_warn(shell, "Failed to get Reader System Issuer CA public key: %d\n", error.ToInt()));

	DoorLock::Utils::HexStringBuffer<CryptoTypes::PublicKey> hexString{};
	VerifyOrReturnValue(DoorLock::Utils::ArrayToHexString(hexString, publicKey), -EINVAL,
			    shell_warn(shell, "Reader System Issuer CA public key hex string invalid!\n"));
	shell_print(shell, "Reader System Issuer CA public key: %s", hexString.data());
	return 0;
}

int HandleIssuerPublicKeySet(const struct shell *shell, size_t argc, char **argv)
{
	VerifyOrReturnValue(IsShellInitialized(), -EIO, shell_warn(shell, "Shell not initialized\n"));

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

	const auto error = Storage::Reader::SetIssuerPublicKey(publicKey);
	VerifyOrReturnValue(error == ALIRO_NO_ERROR, -EIO,
			    shell_warn(shell, "Failed to set Reader System Issuer CA public key: %d\n", error.ToInt()));
	return 0;
}

int HandleIssuerPublicKeyClear(const struct shell *shell, size_t, char **)
{
	VerifyOrReturnValue(IsShellInitialized(), -EIO, shell_warn(shell, "Shell not initialized\n"));

	const auto error = Storage::Reader::ClearIssuerPublicKey();
	VerifyOrReturnValue(error == ALIRO_NO_ERROR, -EIO,
			    shell_warn(shell, "Failed to clear Reader System Issuer CA public key: %d\n",
				       error.ToInt()));
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

} // namespace DoorLock::ReaderShell
