/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <reader_storage/reader.h>

#include "group.h"
#include "reader_psa_ps_ids.h"

#include <aliro/memory.h>
#include <aliro/utils.h>
#include <crypto_utils/crypto_utils.h>
#include <psa/protected_storage.h>
#include <zephyr/logging/log.h>

LOG_MODULE_DECLARE(reader_storage, CONFIG_DOOR_LOCK_ALIRO_READER_STORAGE_LOG_LEVEL);

namespace DoorLock::ReaderStorage {

using namespace Aliro;

AliroError ClearGroup()
{
	return ClearGroupResolvingKey();
}

bool IsGroupResolvingKeySet()
{
	return CryptoUtils::IsKeyAvailable(kGroupResolvingKeyId) == ALIRO_NO_ERROR;
}

psa_key_id_t GetGroupResolvingKeyId()
{
	return kGroupResolvingKeyId;
}

AliroError GetGroupResolvingKey(CryptoTypes::GroupResolvingKey &groupResolvingKey)
{
	const auto error =
		CryptoUtils::ExportKey(kGroupResolvingKeyId, groupResolvingKey.data(), groupResolvingKey.size());
	VerifyOrReturnValue(error == ALIRO_NO_ERROR, error, LOG_ERR("Failed to export Group Resolving Key"));

	return ALIRO_NO_ERROR;
}

AliroError SetGroupResolvingKey(const CryptoTypes::GroupResolvingKey &groupResolvingKey)
{
	CryptoTypes::KeyId keyId{ kGroupResolvingKeyId };
	const auto error = CryptoUtils::ImportGroupResolvingKey(groupResolvingKey, true, keyId);
	VerifyOrReturnValue(error == ALIRO_NO_ERROR, error, LOG_ERR("Failed to import Group Resolving Key"));

	return ALIRO_NO_ERROR;
}

AliroError ClearGroupResolvingKey()
{
	if (!IsGroupResolvingKeySet()) {
		return ALIRO_NO_ERROR;
	}

	CryptoTypes::KeyId keyId{ kGroupResolvingKeyId };
	const auto error = CryptoUtils::DestroyKey(keyId);
	VerifyOrReturnValue(error == ALIRO_NO_ERROR, error, LOG_ERR("Failed to destroy Group Resolving Key"));

	return ALIRO_NO_ERROR;
}

} // namespace DoorLock::ReaderStorage
