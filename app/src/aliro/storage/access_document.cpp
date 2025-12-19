/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifdef CONFIG_DOOR_LOCK_STEP_UP_PHASE

#include "access_document.h"

#include "access_manager_impl.h"
#include "aliro/utils.h"
#include "storage.h"
#include "storage_keys.h"

#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(access_document, CONFIG_DOOR_LOCK_APP_LOG_LEVEL);

namespace Aliro {

namespace {

StorageKeys::KeyNameBuffer GetStorageKeyName(size_t index)
{
	return KeyValueStorage::GetStorageKeyName(StorageKeys::kStorageKeyNameAccessDocument, index);
}

int ReadAccessDocumentHelper(size_t index, AccessDocument &ad)
{
	const auto keyName = GetStorageKeyName(index);
	return KeyValueStorage::Instance().Get(keyName.data(), reinterpret_cast<uint8_t *>(&ad),
					       sizeof(AccessDocument));
}

} // namespace

AliroError LoadAccessDocuments()
{
	AccessDocument ad;
	for (size_t index = 0; index < CONFIG_DOOR_LOCK_STORAGE_MAX_STORED_ACCESS_DOCUMENTS; index++) {
		const auto error = ReadAccessDocumentHelper(index, ad);
		if (error == -ENODATA) {
			continue;
		}

		VerifyOrReturnStatus(error == 0, AliroError::FromInt(error),
				     LOG_ERR("Failed to read Access Document at index: %u, error code: %d", index,
					     error));

		ReturnErrorOnFailure(AccessManagerInstance().AddPublicKey(
			ad.mPublicKey, AccessManager::PublicKeyType::AccessDocument, index));

		LOG_DBG("Loaded AD at index: %u, Version: %u, CI index: %u, Timestamp: %.*s, Access Iteration: %" PRIu64,
			index, ad.mVersion, ad.mCredentialIssuerKeyIndex, ad.mSignedTimestamp.size(),
			ad.mSignedTimestamp.data(), ad.mAccessIteration);
	}

	return ALIRO_NO_ERROR;
}

AliroError StoreAccessDocument(size_t index, const AccessDocument &ad)
{
	const auto keyName = GetStorageKeyName(index);
	const auto error = KeyValueStorage::Instance().Save(keyName.data(), reinterpret_cast<const uint8_t *>(&ad),
							    sizeof(AccessDocument));
	VerifyOrReturnStatus(error == 0, AliroError::FromInt(error),
			     LOG_ERR("Failed to store Access Document at index: %u", index));

	return ALIRO_NO_ERROR;
}

AliroError ReadAccessDocument(size_t index, AccessDocument &ad)
{
	const auto error = ReadAccessDocumentHelper(index, ad);
	VerifyOrReturnStatus(error == 0, AliroError::FromInt(error),
			     LOG_ERR("Failed to get Access Document at index: %u", index));

	return ALIRO_NO_ERROR;
}

AliroError ClearAccessDocument(size_t index)
{
	const auto keyName = GetStorageKeyName(index);
	const auto error = KeyValueStorage::Instance().Clear(keyName.data());
	VerifyOrReturnStatus(error == 0, AliroError::FromInt(error),
			     LOG_ERR("Failed to clear Access Document at index: %u", index));

	return ALIRO_NO_ERROR;
}

} // namespace Aliro

#endif // CONFIG_DOOR_LOCK_STEP_UP_PHASE
