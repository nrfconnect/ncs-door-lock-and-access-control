/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <reader_storage/reader.h>

#include "shell.h"

#include <aliro/utils.h>
#include <zephyr/shell/shell.h>
#include <zephyr/sys/util.h>

#include <cstring>

namespace DoorLock::ReaderStorage {

namespace {

using namespace Aliro;

int HandleIdentifierGet(const struct shell *shell, size_t offset, size_t length)
{
	VerifyOrReturnValue(IsInitialized(), -EIO, shell_warn(shell, "Reader storage not initialized\n"));

	if (!ReaderStorage::IsIdentifierSet()) {
		shell_warn(shell, "Reader Identifier is not set\n");
		return 0;
	}

	Identifier identifier{};
	const auto error = ReaderStorage::GetIdentifier(identifier);
	VerifyOrReturnValue(error == ALIRO_NO_ERROR, -EIO);

	char identifierHex[kReaderIdentifierLength * 2 + 1];
	const size_t convertedLength =
		bin2hex(identifier.data(), identifier.size(), identifierHex, std::size(identifierHex));
	VerifyOrReturnValue(convertedLength == identifier.size() * 2, -EINVAL,
			    shell_warn(shell, "Invalid Reader Identifier hex string!\n"));

	shell_print(shell, "%.*s", static_cast<int>(length * 2), identifierHex + offset * 2);
	return 0;
}

int HandleIdentifierSet(const struct shell *shell, const char *value, size_t offset, size_t length)
{
	VerifyOrReturnValue(IsInitialized(), -EIO, shell_warn(shell, "Reader storage not initialized\n"));

	Identifier identifier{};

	if (ReaderStorage::IsIdentifierSet()) {
		const auto error = ReaderStorage::GetIdentifier(identifier);
		VerifyOrReturnValue(error == ALIRO_NO_ERROR, -EIO,
				    shell_warn(shell, "Failed to get Reader Identifier: %d\n", error.ToInt()));
	}

	size_t argLength = strlen(value);
	VerifyOrReturnValue(argLength == length * 2, -EINVAL, shell_warn(shell, "Invalid Reader Identifier length!\n"));

	const size_t decoded = hex2bin(value, argLength, identifier.data() + offset, length);
	VerifyOrReturnValue(decoded == length, -EINVAL, shell_warn(shell, "Invalid Reader Identifier hex string!\n"));

	const auto error = ReaderStorage::SetIdentifier(identifier);
	VerifyOrReturnValue(error == ALIRO_NO_ERROR, -EIO,
			    shell_warn(shell, "Cannot update identifier: %d\n", error.ToInt()));

	NotifyDataChanged();
	return 0;
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
	VerifyOrReturnValue(IsInitialized(), -EIO, shell_warn(shell, "Reader storage not initialized\n"));

	if (!ReaderStorage::IsPrivateKeySet()) {
		shell_warn(shell, "Reader private key is not set\n");
		return 0;
	}

	CryptoTypes::PublicKey publicKey{};
	const auto error = ReaderStorage::GetPublicKey(publicKey);
	VerifyOrReturnValue(error == ALIRO_NO_ERROR, -EIO,
			    shell_warn(shell, "Failed to get public key: %d\n", error.ToInt()));

	char publicKeyHex[CryptoTypes::kEccP256PublicKeyLength * 2 + 1];
	const size_t convertedLength =
		bin2hex(publicKey.data(), publicKey.size(), publicKeyHex, std::size(publicKeyHex));
	VerifyOrReturnValue(convertedLength == publicKey.size() * 2, -EINVAL,
			    shell_warn(shell, "Invalid public key hex string!\n"));

	shell_print(shell, "Reader public key: %s", publicKeyHex);
	return 0;
}

int HandlePrivateKeySet(const struct shell *shell, size_t argc, char **argv)
{
	VerifyOrReturnValue(IsInitialized(), -EIO, shell_warn(shell, "Reader storage not initialized\n"));

	VerifyOrReturnValue(argc == 2, -EINVAL, shell_warn(shell, "Usage: reader private_key set <64-hex-chars>\n"));

	constexpr size_t kPrivateKeyStringLength{ 2 * CryptoTypes::kEccP256KeyPrivateKeyLength };
	const char *keyStr = argv[1];
	const size_t keyLength = strlen(keyStr);
	VerifyOrReturnValue(keyLength == kPrivateKeyStringLength, -EINVAL,
			    shell_warn(shell, "Invalid key length (must be %zu hex chars)\n", kPrivateKeyStringLength));

	CryptoTypes::PrivateKey privateKey{};
	const size_t decoded = hex2bin(keyStr, keyLength, privateKey.data(), privateKey.size());
	VerifyOrReturnValue(decoded == privateKey.size(), -EINVAL, shell_warn(shell, "Invalid key hex string!\n"));

	if (ReaderStorage::IsPrivateKeySet()) {
		const auto error = ReaderStorage::ClearPrivateKey();
		VerifyOrReturnValue(error == ALIRO_NO_ERROR, -EIO,
				    shell_warn(shell, "Failed to clear private key: %d\n", error.ToInt()));
	}

	const auto error = ReaderStorage::SetPrivateKey(privateKey);
	VerifyOrReturnValue(error == ALIRO_NO_ERROR, -EIO,
			    shell_warn(shell, "Failed to import private key: %d\n", error.ToInt()));

	NotifyDataChanged();
	return 0;
}

int HandlePrivateKeyClear(const struct shell *shell, size_t, char **)
{
	VerifyOrReturnValue(IsInitialized(), -EIO, shell_warn(shell, "Reader storage not initialized\n"));

	const auto error = ReaderStorage::ClearPrivateKey();
	VerifyOrReturnValue(error == ALIRO_NO_ERROR, -EIO,
			    shell_warn(shell, "Failed to clear private key: %d\n", error.ToInt()));

	NotifyDataChanged();
	return 0;
}

} // namespace

SHELL_STATIC_SUBCMD_SET_CREATE(private_key_cmd, SHELL_CMD(list, NULL, "List Reader private key", HandlePrivateKeyList),
			       SHELL_CMD(set, NULL, "Set Reader private key", HandlePrivateKeySet),
			       SHELL_CMD(clear, NULL, "Clear Reader private key", HandlePrivateKeyClear),
			       SHELL_SUBCMD_SET_END);
SHELL_SUBCMD_ADD((reader), private_key, &private_key_cmd, "Reader private key commands", NULL, 0, 0);

SHELL_SUBCMD_ADD((reader), identifier, NULL, "Get/set Reader identifier", HandleIdentifier, 0, 0);
SHELL_SUBCMD_ADD((reader), group_id, NULL, "Get/set Reader group identifier", HandleGroupId, 0, 0);
SHELL_SUBCMD_ADD((reader), group_sub_id, NULL, "Get/set Reader group sub-identifier", HandleGroupSubId, 0, 0);

} // namespace DoorLock::ReaderStorage
