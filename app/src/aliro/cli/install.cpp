/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "shell_private.h"

#include <aliro/aliro.h>
#include <aliro/utils.h>

#include "../aliro_state_control.h"
#include "aliro/utils/hex_string.h"
#include "reader_cache.h"
#include "storage.h"
#include "storage_keys.h"

#include <zephyr/shell/shell.h>
#include <zephyr/sys/util.h>

namespace {

constexpr char kCmdReaderIdentifier[] = "identifier";
constexpr char kCmdReaderGroupIdentifier[] = "group_id";
constexpr char kCmdReaderGroupSubIdentifier[] = "group_sub_id";

int ShellCmdHandleIdentifiers(const struct shell *shell, size_t argc, char **argv)
{
	VerifyOrReturnStatus(IN_RANGE(argc, 1, 2), -EINVAL, shell_warn(shell, "Invalid number of arguments!\n"));
	VerifyOrReturnValue(IsShellInitialized(), -EIO, shell_warn(shell, "Not initialized yet\n"));

	Aliro::Identifier identifier{};
	int status = KeyValueStorage::Instance().Get(Aliro::StorageKeys::kStorageKeyNameIdentifier, identifier.data(),
						     identifier.size());

	size_t offset{ 0 };
	size_t length{ 0 };

	if (CmdMatch(argv[0], kCmdReaderIdentifier)) {
		offset = 0;
		length = identifier.size();
	} else if (CmdMatch(argv[0], kCmdReaderGroupIdentifier)) {
		offset = 0;
		length = Aliro::kReaderGroupIdentifierLength;
	} else if (CmdMatch(argv[0], kCmdReaderGroupSubIdentifier)) {
		offset = Aliro::kReaderGroupIdentifierLength;
		length = Aliro::kReaderGroupSubIdentifierLength;
	} else {
		return -EINVAL;
	}

	if (argc == 1) {
		VerifyOrReturnStatus(status == 0, status,
				     shell_warn(shell, "Cannot get %s, error: %d\n",
						Aliro::StorageKeys::kStorageKeyNameIdentifier, status));

		DoorLock::Utils::HexStringBuffer<Aliro::Identifier> identifierHex{};
		VerifyOrReturnStatus(DoorLock::Utils::ArrayToHexString(identifierHex, identifier), -EINVAL,
				     shell_warn(shell, "Cannot format %s\n", argv[0]));
		shell_print(shell, "%.*s", static_cast<int>(length * 2), identifierHex.data() + offset * 2);
		return 0;
	}

	const size_t argLength = strlen(argv[1]);
	VerifyOrReturnStatus(argLength == length * 2, -EINVAL, shell_warn(shell, "Invalid %s length!\n", argv[0]));

	hex2bin(argv[1], argLength, identifier.data() + offset, length);

	VerifyOrReturnStatus(!KeyValueStorage::Instance().Save(Aliro::StorageKeys::kStorageKeyNameIdentifier,
							       identifier.data(), identifier.size()),
			     -EINVAL,
			     shell_warn(shell, "Cannot update %s\n", Aliro::StorageKeys::kStorageKeyNameIdentifier));

	AliroError aliroError = Aliro::ReaderCache::Instance().SetIdentifier(identifier);
	VerifyOrReturnStatus(aliroError == ALIRO_NO_ERROR, -EINVAL,
			     shell_warn(shell, "Failed to set reader identifier\n"));

	AliroError err = DoorLock::AliroStateControl::UpdateAliroState();
	VerifyOrReturnValue(err == ALIRO_NO_ERROR, -EIO,
			    shell_warn(shell, "Failed to update Aliro state: %d\n", err.ToInt()));

	return 0;
}

SHELL_STATIC_SUBCMD_SET_CREATE(
	install_cmd,
	SHELL_CMD(identifier, NULL,
		  "Set or get reader identifier\n"
		  "  Usage: dl install identifier <32-byte reader_identifier in hex without 0x>",
		  ShellCmdHandleIdentifiers),
	SHELL_CMD(group_id, NULL,
		  "Set or get group ID\n"
		  "  Usage: dl install group_id <16-byte reader_group_identifier in hex without 0x>",
		  ShellCmdHandleIdentifiers),
	SHELL_CMD(group_sub_id, NULL,
		  "Set or get group sub ID\n"
		  "  Usage: dl install group_sub_id <16-byte reader_group_sub_identifier in hex without 0x>",
		  ShellCmdHandleIdentifiers),
	SHELL_SUBCMD_SET_END);

} // namespace

SHELL_SUBCMD_ADD((dl), install, &install_cmd, "Installation commands", NULL, 0, 0);
