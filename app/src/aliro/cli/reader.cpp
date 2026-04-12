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
SHELL_SUBCMD_ADD((reader), private_key, &private_key_cmd, "Reader private key commands", NULL, 0, 0);

SHELL_SUBCMD_ADD((reader), identifier, NULL, "Get/set Reader identifier", HandleIdentifier, 0, 0);
SHELL_SUBCMD_ADD((reader), group_id, NULL, "Get/set Reader group identifier", HandleGroupId, 0, 0);
SHELL_SUBCMD_ADD((reader), group_sub_id, NULL, "Get/set Reader group sub-identifier", HandleGroupSubId, 0, 0);

#ifdef CONFIG_DOOR_LOCK_BLE_UWB
SHELL_STATIC_SUBCMD_SET_CREATE(group_resolving_key_cmd,
			       SHELL_CMD(list, NULL, "List Group Resolving Key", HandleGroupResolvingKeyList),
			       SHELL_CMD(set, NULL, "Set Group Resolving Key", HandleGroupResolvingKeySet),
			       SHELL_CMD(clear, NULL, "Clear Group Resolving Key", HandleGroupResolvingKeyClear),
			       SHELL_SUBCMD_SET_END);
SHELL_SUBCMD_ADD((reader), group_resolving_key, &group_resolving_key_cmd, "Group resolving key commands", NULL, 0, 0);
#endif // CONFIG_DOOR_LOCK_BLE_UWB

} // namespace DoorLock::ReaderShell
