/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "shell_private.h"

#include <aliro/crypto_key_ids.h>
#include <aliro/memory.h>
#include <aliro/utils.h>

#include "aliro/utils/hex_string.h"

#include <zephyr/shell/shell.h>

#include <cstdlib>

namespace {

int ParseIndex(const struct shell *shell, const char *indexStr, size_t &index)
{
	char *endPtr{ nullptr };
	errno = 0;
	unsigned long val = strtoul(indexStr, &endPtr, 10);
	VerifyOrReturnStatus(errno == 0 && endPtr != indexStr, -EINVAL, shell_warn(shell, "Invalid index!\n"));

	index = val;
	return 0;
}

int PrintKpersistentKeys(const struct shell *shell, Aliro::CryptoTypes::KeyId *kpersistentKeyIds,
			 size_t kpersistentCount)
{
	shell_print(shell, "Number of Kpersistent keys: %u", kpersistentCount);
	shell_print(shell, "Index   ID          Public Key");
	shell_print(shell, "--------------------------------");
	for (size_t i = 0; i < kpersistentCount; i++) {
		const Aliro::CryptoTypes::KeyId kpersistentKeyId = kpersistentKeyIds[i];
		const size_t kpersistentKeyIndex = kpersistentKeyId - Aliro::kKpersistentRangeBegin;

		Aliro::CryptoTypes::PublicKey publicKey{};
		auto *kpersistentManager = GetShellKpersistentManager();
		VerifyOrReturnValue(kpersistentManager->GetAccessCredentialPublicKey(kpersistentKeyId, publicKey) ==
					    ALIRO_NO_ERROR,
				    -EINVAL, shell_warn(shell, "Cannot get Access Credential public key\n"));
		DoorLock::Utils::HexStringBuffer<Aliro::CryptoTypes::PublicKey> hexString{};
		VerifyOrReturnValue(DoorLock::Utils::ArrayToHexString(hexString, publicKey), -EINVAL,
				    shell_warn(shell, "Cannot convert Access Credential public key to hex\n"));
		shell_print(shell, "%-2u      0x%08x  %s", kpersistentKeyIndex, kpersistentKeyId, hexString.data());
	}

	return 0;
}

int ShellCmdHandleKpersistentList(const struct shell *shell, size_t argc, char **argv)
{
	VerifyOrReturnValue(IsShellInitialized(), -EIO, shell_warn(shell, "Not initialized yet\n"));
	auto *kpersistentManager = GetShellKpersistentManager();
	VerifyOrReturnValue(kpersistentManager, -EIO, shell_warn(shell, "Kpersistent manager not initialized\n"));

	VerifyOrReturnValue(argc == 1, -EINVAL, shell_warn(shell, "Invalid number of arguments!\n"));

	size_t kpersistentCount{};
	VerifyOrReturnValue(kpersistentManager->GetKpersistentCount(kpersistentCount) == ALIRO_NO_ERROR, -EINVAL,
			    shell_warn(shell, "Cannot get Kpersistent count\n"));
	VerifyOrReturnValue(kpersistentCount > 0, 0, shell_print(shell, "No Kpersistent keys found\n"));

	auto kpersistentKeyIds = Aliro::make_unique_array_nothrow<Aliro::CryptoTypes::KeyId>(kpersistentCount);
	VerifyOrReturnValue(kpersistentKeyIds, -ENOMEM,
			    shell_warn(shell, "Cannot allocate memory for Kpersistent key IDs\n"));

	VerifyOrReturnValue(kpersistentManager->GetKpersistentKeyIds(kpersistentKeyIds.get(), kpersistentCount) ==
				    ALIRO_NO_ERROR,
			    -EINVAL, shell_warn(shell, "Cannot get Kpersistent key IDs\n"));

	return PrintKpersistentKeys(shell, kpersistentKeyIds.get(), kpersistentCount);
}

int ShellCmdHandleKpersistentClear(const struct shell *shell, size_t argc, char **argv)
{
	VerifyOrReturnValue(IsShellInitialized(), -EIO, shell_warn(shell, "Not initialized yet\n"));
	auto *kpersistentManager = GetShellKpersistentManager();
	VerifyOrReturnValue(kpersistentManager, -EIO, shell_warn(shell, "Kpersistent manager not initialized\n"));

	VerifyOrReturnValue(argc == 2, -EINVAL, shell_warn(shell, "Invalid number of arguments!\n"));

	if (strcmp(argv[1], "all") == 0) {
		shell_print(shell, "Removing all Kpersistent keys");
		kpersistentManager->RemoveAllKpersistent();
		return 0;
	}

	size_t index{};
	VerifyOrReturnValue(ParseIndex(shell, argv[1], index) == 0, -EINVAL);
	shell_print(shell, "Removing Kpersistent key with index: %u", index);
	VerifyOrReturnValue(kpersistentManager->RemoveKpersistent(index) == ALIRO_NO_ERROR, -EINVAL,
			    shell_warn(shell, "Cannot remove Kpersistent key\n"));
	return 0;
}

SHELL_STATIC_SUBCMD_SET_CREATE(kpersistent_cmd,
			       SHELL_CMD(list, NULL,
					 "List Kpersistent keys\n"
					 "  Usage: dl kpersistent list",
					 ShellCmdHandleKpersistentList),
			       SHELL_CMD(clear, NULL,
					 "Clear Kpersistent key\n"
					 "  Usage: dl kpersistent clear <index>\n"
					 "       dl kpersistent clear all",
					 ShellCmdHandleKpersistentClear),
			       SHELL_SUBCMD_SET_END);

} // namespace

SHELL_SUBCMD_ADD((dl), kpersistent, &kpersistent_cmd, "Manage Kpersistent keys", NULL, 0, 0);
