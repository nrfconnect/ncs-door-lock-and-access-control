/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "shell_private.h"

#include "reader.h"

#include "aliro/aliro_state_control.h"
#include "aliro/utils.h"
#include "aliro/utils/hex_string.h"

#ifdef CONFIG_DOOR_LOCK_READER_CERTIFICATE
#include "storage_keys.h"
#endif // CONFIG_DOOR_LOCK_READER_CERTIFICATE

#include <algorithm>
#include <array>
#include <zephyr/shell/shell.h>
#include <zephyr/sys/util.h>

namespace DoorLock::ReaderShell {
namespace {

using namespace Aliro;

int UpdateAliroState(const struct shell *shell)
{
	const auto error = DoorLock::AliroStateControl::UpdateAliroState();
	VerifyOrReturnValue(error == ALIRO_NO_ERROR, -EIO,
			    shell_warn(shell, "Failed to update Aliro state: %d\n", error.ToInt()));
	return 0;
}

int HandleIdentifierGet(const struct shell *shell, size_t offset, size_t length)
{
	VerifyOrReturnValue(IsShellInitialized(), -EIO, shell_warn(shell, "Shell not initialized\n"));

	if (!Storage::Reader::IsIdentifierSet()) {
		shell_warn(shell, "Reader Identifier is not set\n");
		return 0;
	}

	Identifier identifier{};
	const auto error = Storage::Reader::GetIdentifier(identifier);
	VerifyOrReturnValue(error == ALIRO_NO_ERROR, -EIO);

	DoorLock::Utils::HexStringBuffer<Identifier> identifierHex{};
	VerifyOrReturnValue(DoorLock::Utils::ArrayToHexString(identifierHex, identifier), -EINVAL,
			    shell_warn(shell, "Cannot format Reader Identifier\n"));
	shell_print(shell, "%.*s", static_cast<int>(length * 2), identifierHex.data() + offset * 2);
	return 0;
}

int HandleIdentifierSet(const struct shell *shell, const char *value, size_t offset, size_t length)
{
	VerifyOrReturnValue(IsShellInitialized(), -EIO, shell_warn(shell, "Shell not initialized\n"));

	Identifier identifier{};

	if (Storage::Reader::IsIdentifierSet()) {
		const auto error = Storage::Reader::GetIdentifier(identifier);
		VerifyOrReturnValue(error == ALIRO_NO_ERROR, -EIO,
				    shell_warn(shell, "Failed to get Reader Identifier: %d\n", error.ToInt()));
	}

	size_t argLength = strlen(value);
	VerifyOrReturnValue(argLength == length * 2, -EINVAL, shell_warn(shell, "Invalid Reader Identifier length!\n"));

	const size_t decoded = hex2bin(value, argLength, identifier.data() + offset, length);
	VerifyOrReturnValue(decoded == length, -EINVAL, shell_warn(shell, "Invalid Reader Identifier hex string!\n"));

	const auto error = Storage::Reader::SetIdentifier(identifier);
	VerifyOrReturnValue(error == ALIRO_NO_ERROR, -EIO,
			    shell_warn(shell, "Cannot update identifier: %d\n", error.ToInt()));

	return UpdateAliroState(shell);
}

int HandleIdentifier(const struct shell *shell, size_t argc, char **argv)
{
	if (argc == 1) {
		return HandleIdentifierGet(shell, 0, kReaderIdentifierLength);
	}
	VerifyOrReturnValue(argc == 2, -EINVAL, shell_warn(shell, "Usage: reader identifier [32-byte-hex]\n"));
	return HandleIdentifierSet(shell, argv[1], 0, kReaderIdentifierLength);
}

int HandleGroupId(const struct shell *shell, size_t argc, char **argv)
{
	if (argc == 1) {
		return HandleIdentifierGet(shell, 0, kReaderGroupIdentifierLength);
	}
	VerifyOrReturnValue(argc == 2, -EINVAL, shell_warn(shell, "Usage: reader group_id [16-byte-hex]\n"));
	return HandleIdentifierSet(shell, argv[1], 0, kReaderGroupIdentifierLength);
}

int HandleGroupSubId(const struct shell *shell, size_t argc, char **argv)
{
	if (argc == 1) {
		return HandleIdentifierGet(shell, kReaderGroupIdentifierLength, kReaderGroupSubIdentifierLength);
	}
	VerifyOrReturnValue(argc == 2, -EINVAL, shell_warn(shell, "Usage: reader group_sub_id [16-byte-hex]\n"));
	return HandleIdentifierSet(shell, argv[1], kReaderGroupIdentifierLength, kReaderGroupSubIdentifierLength);
}

int HandlePrivateKeyList(const struct shell *shell, size_t, char **)
{
	VerifyOrReturnValue(IsShellInitialized(), -EIO, shell_warn(shell, "Shell not initialized\n"));

	if (!Storage::Reader::IsPrivateKeySet()) {
		shell_warn(shell, "Reader private key is not set");
		return 0;
	}

	CryptoTypes::PublicKey publicKey{};
	const auto error = Storage::Reader::GetPublicKey(publicKey);
	VerifyOrReturnValue(error == ALIRO_NO_ERROR, -EIO,
			    shell_warn(shell, "Failed to get public key: %d\n", error.ToInt()));

	DoorLock::Utils::HexStringBuffer<CryptoTypes::PublicKey> hexString{};
	VerifyOrReturnValue(DoorLock::Utils::ArrayToHexString(hexString, publicKey), -EINVAL,
			    shell_warn(shell, "Invalid public key hex string!\n"));

	shell_print(shell, "Reader public key: %s", hexString.data());
	return 0;
}

int HandlePrivateKeySet(const struct shell *shell, size_t argc, char **argv)
{
	VerifyOrReturnValue(IsShellInitialized(), -EIO, shell_warn(shell, "Shell not initialized\n"));

	VerifyOrReturnValue(argc == 2, -EINVAL, shell_warn(shell, "Usage: reader private_key set <64-hex-chars>\n"));

	constexpr size_t kPrivateKeyStringLength{ 2 * CryptoTypes::kEccP256KeyPrivateKeyLength };
	const char *keyStr = argv[1];
	const size_t keyLength = strlen(keyStr);
	VerifyOrReturnValue(keyLength == kPrivateKeyStringLength, -EINVAL,
			    shell_warn(shell, "Invalid key length (must be %zu hex chars)\n", kPrivateKeyStringLength));

	CryptoTypes::PrivateKey privateKey{};
	const size_t decoded = hex2bin(keyStr, keyLength, privateKey.data(), privateKey.size());
	VerifyOrReturnValue(decoded == privateKey.size(), -EINVAL, shell_warn(shell, "Invalid key hex string!\n"));

	if (Storage::Reader::IsPrivateKeySet()) {
		const auto error = Storage::Reader::ClearPrivateKey();
		VerifyOrReturnValue(error == ALIRO_NO_ERROR, -EIO,
				    shell_warn(shell, "Failed to clear private key: %d\n", error.ToInt()));
	}

	const auto error = Storage::Reader::SetPrivateKey(privateKey);
	VerifyOrReturnValue(error == ALIRO_NO_ERROR, -EIO,
			    shell_warn(shell, "Failed to import private key: %d\n", error.ToInt()));

	return UpdateAliroState(shell);
}

int HandlePrivateKeyClear(const struct shell *shell, size_t, char **)
{
	VerifyOrReturnValue(IsShellInitialized(), -EIO, shell_warn(shell, "Shell not initialized\n"));

	const auto error = Storage::Reader::ClearPrivateKey();
	VerifyOrReturnValue(error == ALIRO_NO_ERROR, -EIO,
			    shell_warn(shell, "Failed to clear private key: %d\n", error.ToInt()));

	return UpdateAliroState(shell);
}

#ifdef CONFIG_DOOR_LOCK_READER_CERTIFICATE

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

#endif // CONFIG_DOOR_LOCK_READER_CERTIFICATE

#ifdef CONFIG_DOOR_LOCK_BLE_UWB

int HandleGroupResolvingKeyList(const struct shell *shell, size_t, char **)
{
	VerifyOrReturnValue(IsShellInitialized(), -EIO, shell_warn(shell, "Shell not initialized\n"));

	CryptoTypes::GroupResolvingKey groupResolvingKey{};
	const auto error = Storage::Reader::GetGroupResolvingKey(groupResolvingKey);
	if (error == ALIRO_INVALID_STATE) {
		shell_warn(shell, "Group resolving key not set");
		return 0;
	}
	VerifyOrReturnValue(error == ALIRO_NO_ERROR, -EIO,
			    shell_warn(shell, "Failed to get group resolving key: %d\n", error.ToInt()));

	DoorLock::Utils::HexStringBuffer<CryptoTypes::GroupResolvingKey> hexString{};
	VerifyOrReturnValue(DoorLock::Utils::ArrayToHexString(hexString, groupResolvingKey), -EINVAL,
			    shell_warn(shell, "Invalid group resolving key hex string!\n"));
	shell_print(shell, "Group resolving key (%zu bytes): %s", groupResolvingKey.size(), hexString.data());
	return 0;
}

int HandleGroupResolvingKeySet(const struct shell *shell, size_t argc, char **argv)
{
	VerifyOrReturnValue(IsShellInitialized(), -EIO, shell_warn(shell, "Shell not initialized\n"));

	VerifyOrReturnValue(argc == 2, -EINVAL,
			    shell_warn(shell, "Usage: reader group_resolving_key set <32-hex-chars>\n"));

	constexpr size_t kGroupResolvingKeyStringLength{ 2 * CryptoTypes::kGroupResolvingKeyLength };
	const char *keyString = argv[1];
	const size_t keyStringLength = strlen(keyString);
	VerifyOrReturnValue(keyStringLength == kGroupResolvingKeyStringLength, -EINVAL,
			    shell_warn(shell, "Invalid key length (must be %zu hex chars)\n",
				       kGroupResolvingKeyStringLength));

	CryptoTypes::GroupResolvingKey groupResolvingKey{};
	const size_t decoded = hex2bin(keyString, keyStringLength, groupResolvingKey.data(), groupResolvingKey.size());
	VerifyOrReturnValue(decoded == groupResolvingKey.size(), -EINVAL,
			    shell_warn(shell, "Invalid group resolving key hex string!\n"));

	if (Storage::Reader::IsGroupResolvingKeySet()) {
		const auto error = Storage::Reader::ClearGroupResolvingKey();
		VerifyOrReturnValue(error == ALIRO_NO_ERROR, -EIO,
				    shell_warn(shell, "Failed to clear group resolving key: %d\n", error.ToInt()));
	}

	const auto error = Storage::Reader::SetGroupResolvingKey(groupResolvingKey);
	VerifyOrReturnValue(error == ALIRO_NO_ERROR, -EIO,
			    shell_warn(shell, "Failed to set group resolving key: %d\n", error.ToInt()));
	return 0;
}

int HandleGroupResolvingKeyClear(const struct shell *shell, size_t, char **)
{
	VerifyOrReturnValue(IsShellInitialized(), -EIO, shell_warn(shell, "Shell not initialized\n"));

	const auto error = Storage::Reader::ClearGroupResolvingKey();
	VerifyOrReturnValue(error == ALIRO_NO_ERROR, -EIO,
			    shell_warn(shell, "Failed to clear group resolving key: %d\n", error.ToInt()));
	return 0;
}

#endif // CONFIG_DOOR_LOCK_BLE_UWB

} // namespace

SHELL_STATIC_SUBCMD_SET_CREATE(private_key_cmd, SHELL_CMD(list, NULL, "List Reader private key", HandlePrivateKeyList),
			       SHELL_CMD(set, NULL, "Set Reader private key", HandlePrivateKeySet),
			       SHELL_CMD(clear, NULL, "Clear Reader private key", HandlePrivateKeyClear),
			       SHELL_SUBCMD_SET_END);

#ifdef CONFIG_DOOR_LOCK_BLE_UWB
SHELL_STATIC_SUBCMD_SET_CREATE(group_resolving_key_cmd,
			       SHELL_CMD(list, NULL, "List Group Resolving Key", HandleGroupResolvingKeyList),
			       SHELL_CMD(set, NULL, "Set Group Resolving Key", HandleGroupResolvingKeySet),
			       SHELL_CMD(clear, NULL, "Clear Group Resolving Key", HandleGroupResolvingKeyClear),
			       SHELL_SUBCMD_SET_END);
#endif // CONFIG_DOOR_LOCK_BLE_UWB

#ifdef CONFIG_DOOR_LOCK_READER_CERTIFICATE
SHELL_STATIC_SUBCMD_SET_CREATE(certificate_cmd, SHELL_CMD(list, NULL, "List Reader certificate", HandleCertificateList),
			       SHELL_CMD(set, NULL, "Set Reader certificate", HandleCertificateSet),
			       SHELL_CMD(clear, NULL, "Clear Reader certificate", HandleCertificateClear),
			       SHELL_SUBCMD_SET_END);

SHELL_STATIC_SUBCMD_SET_CREATE(issuer_public_key_cmd,
			       SHELL_CMD(list, NULL, "List Issuer public key", HandleIssuerPublicKeyList),
			       SHELL_CMD(set, NULL, "Set Issuer public key", HandleIssuerPublicKeySet),
			       SHELL_CMD(clear, NULL, "Clear Issuer public key", HandleIssuerPublicKeyClear),
			       SHELL_SUBCMD_SET_END);
#endif // CONFIG_DOOR_LOCK_READER_CERTIFICATE

SHELL_STATIC_SUBCMD_SET_CREATE(reader_cmd, SHELL_CMD(identifier, NULL, "Get/set Reader identifier", HandleIdentifier),
			       SHELL_CMD(group_id, NULL, "Get/set Reader group identifier", HandleGroupId),
			       SHELL_CMD(group_sub_id, NULL, "Get/set Reader group sub-identifier", HandleGroupSubId),
			       SHELL_CMD(private_key, &private_key_cmd, "Reader private key commands", NULL),
#ifdef CONFIG_DOOR_LOCK_BLE_UWB
			       SHELL_CMD(group_resolving_key, &group_resolving_key_cmd, "Group resolving key commands",
					 NULL),
#endif // CONFIG_DOOR_LOCK_BLE_UWB
#ifdef CONFIG_DOOR_LOCK_READER_CERTIFICATE
			       SHELL_CMD(certificate, &certificate_cmd, "Reader certificate commands", NULL),
			       SHELL_CMD(issuer_public_key, &issuer_public_key_cmd, "Issuer public key commands", NULL),
#endif // CONFIG_DOOR_LOCK_READER_CERTIFICATE
			       SHELL_SUBCMD_SET_END);

SHELL_SUBCMD_ADD((dl), reader, &reader_cmd, "Reader data commands", NULL, 0, 0);

} // namespace DoorLock::ReaderShell
