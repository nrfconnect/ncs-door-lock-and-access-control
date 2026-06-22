/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "external_nvs/external_nvs.h"

#ifdef CONFIG_DOOR_LOCK_EXTERNAL_NVS_ROLLBACK_PROTECTION
#include "counter.h"
#include "storage.h"
#endif // CONFIG_DOOR_LOCK_EXTERNAL_NVS_ROLLBACK_PROTECTION

#include <zephyr/shell/shell.h>
#include <zephyr/sys/util.h>

#include <cstdlib>
#include <cstring>
#include <limits>

namespace {

using namespace DoorLock::ExternalNvs;

constexpr size_t kMaxDataSize{ CONFIG_DOOR_LOCK_EXTERNAL_NVS_MAX_DATA_SIZE };

int ParseId(const struct shell *shell, const char *arg, Id &id)
{
	char *end{};
	unsigned long value = strtoul(arg, &end, 0);

	if (*end != '\0' || value > std::numeric_limits<Id>::max()) {
		shell_error(shell, "Invalid ID: %s", arg);
		return -EINVAL;
	}

	id = static_cast<Id>(value);
	return 0;
}

int CmdRead(const struct shell *shell, size_t argc, char **argv)
{
	if (argc != 2) {
		shell_error(shell, "Usage: ext_nvs read <id>");
		return -EINVAL;
	}

	Id id{};
	int ec = ParseId(shell, argv[1], id);
	if (ec) {
		return ec;
	}

#ifdef CONFIG_DOOR_LOCK_EXTERNAL_NVS_ROLLBACK_PROTECTION
	if (id == kUniqueIdReservedId) {
		shell_error(shell, "Cannot read reserved ID %u", id);
		return -EINVAL;
	}
#endif // CONFIG_DOOR_LOCK_EXTERNAL_NVS_ROLLBACK_PROTECTION

	uint8_t buf[kMaxDataSize];
	size_t len{ sizeof(buf) };

	ec = Read(id, buf, len);
	if (ec) {
		shell_error(shell, "Read failed: %d", ec);
		return ec;
	}

	shell_fprintf(shell, SHELL_NORMAL, "ID %u (%zu bytes):\n", id, len);
	shell_hexdump(shell, buf, len);

	return 0;
}

int CmdWrite(const struct shell *shell, size_t argc, char **argv)
{
	if (argc != 3) {
		shell_error(shell, "Usage: ext_nvs write <id> <hex_data>");
		return -EINVAL;
	}

	Id id{};
	int ec = ParseId(shell, argv[1], id);
	if (ec) {
		return ec;
	}

#ifdef CONFIG_DOOR_LOCK_EXTERNAL_NVS_ROLLBACK_PROTECTION
	if (id == kUniqueIdReservedId) {
		shell_error(shell, "Cannot write to reserved ID %u", id);
		return -EINVAL;
	}
#endif // CONFIG_DOOR_LOCK_EXTERNAL_NVS_ROLLBACK_PROTECTION

	const char *hexStr = argv[2];
	const size_t hexLen = strlen(hexStr);

	if (hexLen % 2 != 0) {
		shell_error(shell, "Hex string must have even length");
		return -EINVAL;
	}

	const size_t dataLen = hexLen / 2;

	if (dataLen == 0 || dataLen > kMaxDataSize) {
		shell_error(shell, "Data length %zu out of range [1, %zu]", dataLen, kMaxDataSize);
		return -EINVAL;
	}

	uint8_t buf[kMaxDataSize];
	size_t decoded = hex2bin(hexStr, hexLen, buf, sizeof(buf));

	if (decoded != dataLen) {
		shell_error(shell, "Invalid hex string");
		return -EINVAL;
	}

	ec = Write(id, buf, dataLen);
	if (ec) {
		shell_error(shell, "Write failed: %d", ec);
		return ec;
	}

	shell_print(shell, "Wrote %zu bytes to ID %u", dataLen, id);
	return 0;
}

int CmdDelete(const struct shell *shell, size_t argc, char **argv)
{
	if (argc != 2) {
		shell_error(shell, "Usage: ext_nvs delete <id>");
		return -EINVAL;
	}

	Id id{};
	int ec = ParseId(shell, argv[1], id);
	if (ec) {
		return ec;
	}

#ifdef CONFIG_DOOR_LOCK_EXTERNAL_NVS_ROLLBACK_PROTECTION
	if (id == kUniqueIdReservedId) {
		shell_error(shell, "Cannot delete reserved ID %u", id);
		return -EINVAL;
	}
#endif // CONFIG_DOOR_LOCK_EXTERNAL_NVS_ROLLBACK_PROTECTION

	ec = Delete(id);
	if (ec) {
		shell_error(shell, "Delete failed: %d", ec);
		return ec;
	}

	shell_print(shell, "Deleted ID %u", id);
	return 0;
}

int CmdClear(const struct shell *shell, size_t argc, char **)
{
	if (argc != 1) {
		shell_error(shell, "Usage: ext_nvs clear");
		return -EINVAL;
	}

	const int ec = Clear();
	if (ec) {
		shell_error(shell, "Clear failed: %d", ec);
		return ec;
	}

	shell_print(shell, "External NVS cleared");
	return 0;
}

int CmdList(const struct shell *shell, size_t argc, char **argv)
{
	if (argc != 3) {
		shell_error(shell, "Usage: ext_nvs list <start_id> <end_id>");
		return -EINVAL;
	}

	Id startId{};
	int ec = ParseId(shell, argv[1], startId);
	if (ec) {
		return ec;
	}

	Id endId{};
	ec = ParseId(shell, argv[2], endId);
	if (ec) {
		return ec;
	}

#ifdef CONFIG_DOOR_LOCK_EXTERNAL_NVS_ROLLBACK_PROTECTION
	if (endId == kUniqueIdReservedId) {
		shell_error(shell, "end_id must be less than reserved ID %u", kUniqueIdReservedId);
		return -EINVAL;
	}
#endif // CONFIG_DOOR_LOCK_EXTERNAL_NVS_ROLLBACK_PROTECTION

	if (startId >= endId) {
		shell_error(shell, "start_id must be less than end_id");
		return -EINVAL;
	}

	for (size_t i = startId; i <= endId; i++) {
		uint8_t buf[kMaxDataSize];
		size_t len{ sizeof(buf) };
		const Id id = static_cast<Id>(i);

		ec = Read(id, buf, len);
		if (ec == 0) {
			shell_fprintf(shell, SHELL_NORMAL, "ID %u (%zu bytes):\n", id, len);
			shell_hexdump(shell, buf, len);
		}
	}

	return 0;
}

#ifdef CONFIG_DOOR_LOCK_EXTERNAL_NVS_ROLLBACK_PROTECTION

void PrintUniqueId(const struct shell *shell, const char *label, const Counter::UniqueId &uniqueId, size_t len)
{
	char hex[2 * sizeof(Counter::UniqueId) + 1];
	size_t hexLen = bin2hex(uniqueId.data(), len, hex, sizeof(hex));

	shell_print(shell, "%s: %.*s", label, static_cast<int>(hexLen), hex);
}

int CmdUniqueId(const struct shell *shell, size_t argc, char **)
{
	if (argc != 1) {
		shell_error(shell, "Usage: ext_nvs unique_id");
		return -EINVAL;
	}

	Counter::UniqueId deviceId{};
	int ec = Counter::GetUniqueId(deviceId);
	if (ec) {
		shell_error(shell, "Failed to get device unique ID: %d", ec);
		return ec;
	}

	Counter::UniqueId storedId{};
	size_t readLen{ storedId.size() };
	ec = Storage::Read(kUniqueIdReservedId, storedId.data(), readLen);
	if (ec) {
		shell_error(shell, "Failed to read stored unique ID: %d", ec);
		return ec;
	}

	PrintUniqueId(shell, "Device ", deviceId, deviceId.size());
	PrintUniqueId(shell, "Storage", storedId, readLen);

	return 0;
}

#endif // CONFIG_DOOR_LOCK_EXTERNAL_NVS_ROLLBACK_PROTECTION

SHELL_STATIC_SUBCMD_SET_CREATE(ext_nvs_cmd, SHELL_CMD(read, NULL, "Read entry: ext_nvs read <id>", CmdRead),
			       SHELL_CMD(write, NULL, "Write entry: ext_nvs write <id> <hex_data>", CmdWrite),
			       SHELL_CMD(delete, NULL, "Delete entry: ext_nvs delete <id>", CmdDelete),
			       SHELL_CMD(clear, NULL, "Clear all entries: ext_nvs clear", CmdClear),
			       SHELL_CMD(list, NULL, "List entries in range: ext_nvs list <start_id> <end_id>",
					 CmdList),
#ifdef CONFIG_DOOR_LOCK_EXTERNAL_NVS_ROLLBACK_PROTECTION
			       SHELL_CMD(unique_id, NULL, "Show device and stored unique IDs", CmdUniqueId),
#endif // CONFIG_DOOR_LOCK_EXTERNAL_NVS_ROLLBACK_PROTECTION
			       SHELL_SUBCMD_SET_END);

} // namespace

SHELL_CMD_REGISTER(ext_nvs, &ext_nvs_cmd, "External NVS commands", NULL);
