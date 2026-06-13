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

int HandleGroupResolvingKeyList(const struct shell *shell, size_t, char **)
{
	VerifyOrReturnValue(IsInitialized(), -EIO, shell_warn(shell, "Reader storage not initialized\n"));

	if (!ReaderStorage::IsGroupResolvingKeySet()) {
		shell_warn(shell, "Group resolving key not set\n");
		return 0;
	}

	CryptoTypes::GroupResolvingKey groupResolvingKey{};
	const auto error = ReaderStorage::GetGroupResolvingKey(groupResolvingKey);
	VerifyOrReturnValue(error == ALIRO_NO_ERROR, -EIO,
			    shell_warn(shell, "Failed to get group resolving key: %d\n", error.ToInt()));

	char hexString[CryptoTypes::kGroupResolvingKeyLength * 2 + 1];
	const size_t convertedLength =
		bin2hex(groupResolvingKey.data(), groupResolvingKey.size(), hexString, std::size(hexString));
	VerifyOrReturnValue(convertedLength == groupResolvingKey.size() * 2, -EINVAL,
			    shell_warn(shell, "Invalid group resolving key hex string!\n"));
	shell_print(shell, "Group resolving key (%zu bytes): %s", groupResolvingKey.size(), hexString);
	return 0;
}

int HandleGroupResolvingKeySet(const struct shell *shell, size_t argc, char **argv)
{
	VerifyOrReturnValue(IsInitialized(), -EIO, shell_warn(shell, "Reader storage not initialized\n"));

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

	if (ReaderStorage::IsGroupResolvingKeySet()) {
		const auto error = ReaderStorage::ClearGroupResolvingKey();
		VerifyOrReturnValue(error == ALIRO_NO_ERROR, -EIO,
				    shell_warn(shell, "Failed to clear group resolving key: %d\n", error.ToInt()));
	}

	const auto error = ReaderStorage::SetGroupResolvingKey(groupResolvingKey);
	VerifyOrReturnValue(error == ALIRO_NO_ERROR, -EIO,
			    shell_warn(shell, "Failed to set group resolving key: %d\n", error.ToInt()));

	NotifyDataChanged();
	return 0;
}

int HandleGroupResolvingKeyClear(const struct shell *shell, size_t, char **)
{
	VerifyOrReturnValue(IsInitialized(), -EIO, shell_warn(shell, "Reader storage not initialized\n"));

	const auto error = ReaderStorage::ClearGroupResolvingKey();
	VerifyOrReturnValue(error == ALIRO_NO_ERROR, -EIO,
			    shell_warn(shell, "Failed to clear group resolving key: %d\n", error.ToInt()));

	NotifyDataChanged();
	return 0;
}

} // namespace

SHELL_STATIC_SUBCMD_SET_CREATE(group_resolving_key_cmd,
			       SHELL_CMD(list, NULL, "List Group Resolving Key", HandleGroupResolvingKeyList),
			       SHELL_CMD(set, NULL, "Set Group Resolving Key", HandleGroupResolvingKeySet),
			       SHELL_CMD(clear, NULL, "Clear Group Resolving Key", HandleGroupResolvingKeyClear),
			       SHELL_SUBCMD_SET_END);
SHELL_SUBCMD_ADD((reader), group_resolving_key, &group_resolving_key_cmd, "Group resolving key commands", NULL, 0, 0);

} // namespace DoorLock::ReaderStorage
