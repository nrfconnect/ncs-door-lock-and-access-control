/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "key.h"

#include <hw_unique_key.h>

#include <zephyr/logging/log.h>

LOG_MODULE_DECLARE(external_nvs, CONFIG_DOOR_LOCK_EXTERNAL_NVS_LOG_LEVEL);

namespace DoorLock::ExternalNvs::Key {

namespace {

#ifdef HUK_HAS_KMU
constexpr hw_unique_key_slot kKeySlot{ HUK_KEYSLOT_MEXT };
#else // HUK_HAS_KMU
constexpr hw_unique_key_slot kKeySlot{ HUK_KEYSLOT_KDR };
#endif // HUK_HAS_KMU

} // namespace

int Init()
{
#ifndef CONFIG_HW_UNIQUE_KEY_WRITE_ON_CRYPTO_INIT
	if (!hw_unique_key_are_any_written()) {
		LOG_ERR("Hardware Unique Keys are not written");
		return -ENODEV;
	}
#endif // CONFIG_HW_UNIQUE_KEY_WRITE_ON_CRYPTO_INIT

	return 0;
}

int Derive(Id id, Key &key)
{
	const auto result = hw_unique_key_derive_key(kKeySlot, nullptr, 0, reinterpret_cast<uint8_t *>(&id), sizeof(id),
						     key.data(), key.size());
	if (result != HW_UNIQUE_KEY_SUCCESS) {
		LOG_ERR("Failed to derive key: %d", result);
		return -EIO;
	}

	return 0;
}

} // namespace DoorLock::ExternalNvs::Key
