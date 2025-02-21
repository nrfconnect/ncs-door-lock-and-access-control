/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <storage.h>

#include <consts.h>
#include <crypto/crypto_common.h>
#include <crypto/crypto_key_storage.h>
#include <reader_identifier/reader_identifier.h>
#include <util/utils.h>

#include <zephyr/fs/nvs.h>
#include <zephyr/settings/settings.h>
#include <zephyr/shell/shell.h>
#include <zephyr/sys/reboot.h>
#include <zephyr/sys/util.h>

#include <array>

namespace {

constexpr size_t kReaderGroupIdentifierStringLength{ 2 * Aliro::kReaderGroupIdentifierLength };
constexpr size_t kAccessCredentialPublicKeyStringLength{ 2 * Aliro::kEccP256PublicKeyLength };

static int ShellCmdHandleGroupAndSubGroupId(const struct shell *shell, size_t argc, char **argv)
{
	std::array<uint8_t, Aliro::kReaderGroupIdentifierLength> groupId{};

	VerifyOrReturnStatus(IN_RANGE(argc, 1, 2), -EINVAL, shell_warn(shell, "Invalid number of arguments!\n"));

	if (argc == 1) {
		int ec = KeyValueStorage::Instance().Get(argv[0], groupId.data(), groupId.size());
		if (!ec) {
			shell_hexdump(shell, groupId.data(), groupId.size());
		}
		return ec;
	}

	size_t len = strlen(argv[1]);
	VerifyOrReturnStatus(len == kReaderGroupIdentifierStringLength, -EINVAL,
			     shell_warn(shell, "Invalid %s length!\n", argv[0]));

	len = hex2bin(argv[1], len, groupId.data(), groupId.size());

	VerifyOrReturnStatus(!KeyValueStorage::Instance().Save(argv[0], groupId.data(), groupId.size()), -EINVAL,
			     shell_warn(shell, "Cannot update %s\n", argv[0]));

	if (!strncmp(argv[0], kStorageKeyNameGroupId, strlen(kStorageKeyNameGroupId))) {
		VerifyOrReturnStatus(Aliro::ReaderIdentifier::Instance().SetGroupIdentifier(groupId) == ALIRO_NO_ERROR,
				     -EIO, shell_warn(shell, "Cannot set %s\n", argv[0]));
	} else if (!strncmp(argv[0], kStorageKeyNameGroupSubId, strlen(kStorageKeyNameGroupSubId))) {
		VerifyOrReturnStatus(Aliro::ReaderIdentifier::Instance().SetSubIdentifier(groupId) == ALIRO_NO_ERROR,
				     -EIO, shell_warn(shell, "Cannot set %s\n", argv[0]));
	}

	return 0;
}

static int ShellCmdHandleProvisioningAccessCredentialKey(const struct shell *shell, size_t argc, char **argv)
{
	Aliro::EccP256PublicKey accessCredentialPublicKey{};

	VerifyOrReturnStatus(IN_RANGE(argc, 1, 2), -EINVAL, shell_warn(shell, "Invalid number of arguments!\n"));

	if (argc == 1) {
		int ec = KeyValueStorage::Instance().Get(argv[0], accessCredentialPublicKey.data(),
							 accessCredentialPublicKey.size());
		if (!ec) {
			shell_hexdump(shell, accessCredentialPublicKey.data(), accessCredentialPublicKey.size());
		}
		return ec;
	}

	size_t len = strlen(argv[1]);
	VerifyOrReturnStatus(len == kAccessCredentialPublicKeyStringLength, -EINVAL,
			     shell_warn(shell, "Invalid %s length!\n", argv[0]));

	len = hex2bin(argv[1], len, accessCredentialPublicKey.data(), accessCredentialPublicKey.size());

	VerifyOrReturnStatus(accessCredentialPublicKey[0] == Aliro::kEccP256PublicKeyPrefix, -EINVAL,
			     shell_warn(shell, "Invalid prefix of the key!\n"));

	VerifyOrReturnStatus(!KeyValueStorage::Instance().Save(argv[0], accessCredentialPublicKey.data(),
							       accessCredentialPublicKey.size()),
			     -EINVAL, shell_warn(shell, "Cannot save key-value in persistent storage!\n"));

	// Update key in CryptoKeyStorage
	VerifyOrReturnStatus(Aliro::CryptoKeyStorage::Instance().SetAccessCredentialPublicKey(Aliro::PublicKey(
				     accessCredentialPublicKey.data(), accessCredentialPublicKey.size())) ==
				     ALIRO_NO_ERROR,
			     -EINVAL, shell_warn(shell, "Cannot update key in CryptoKeyStorage\n"));

	return 0;
}

static int FactoryReset(const struct shell *shell, size_t, char **)
{
	void *storage{ nullptr };
	int status = settings_storage_get(&storage);
	VerifyOrReturnStatus(status == 0, -EIO, shell_warn(shell, "Cannot get storage\n"));
	status = nvs_clear(static_cast<nvs_fs *>(storage));
	VerifyOrReturnStatus(status == 0, -EIO, shell_warn(shell, "Cannot clear storage\n"));
	sys_reboot(SYS_REBOOT_WARM);

	return 0;
}

SHELL_STATIC_SUBCMD_SET_CREATE(
	install_cmd,
	SHELL_CMD(group_id, NULL,
		  "Set or get group ID\n"
		  "  Usage: dl install group_id <16-byte reader_group_identifier in hex without 0x>",
		  ShellCmdHandleGroupAndSubGroupId),
	SHELL_CMD(group_sub_id, NULL,
		  "Set or get group sub ID\n"
		  "  Usage: dl install group_sub_id <16-byte reader_group_sub_identifier in hex in hex without 0x>",
		  ShellCmdHandleGroupAndSubGroupId),
	SHELL_SUBCMD_SET_END);

SHELL_STATIC_SUBCMD_SET_CREATE(provisioning_cmd,
			       SHELL_CMD(AC_key, NULL,
					 "Set or get Access Credential public key\n"
					 "  Usage: dl provisioning AC_key <65-byte public key in hex in hex without 0x>",
					 ShellCmdHandleProvisioningAccessCredentialKey),
			       SHELL_SUBCMD_SET_END);

SHELL_STATIC_SUBCMD_SET_CREATE(door_lock_cmd, SHELL_CMD(install, &install_cmd, "Installation commands", NULL),
			       SHELL_CMD(provisioning, &provisioning_cmd, "Provisioning commands", NULL),
			       SHELL_CMD(factory_reset, NULL, "Factory reset", FactoryReset), SHELL_SUBCMD_SET_END);

SHELL_CMD_REGISTER(dl, &door_lock_cmd, "Door lock commands", NULL);

} // namespace
