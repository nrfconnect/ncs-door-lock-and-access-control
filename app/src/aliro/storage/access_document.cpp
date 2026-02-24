/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "access_document.h"

#include "access_manager_impl.h"
#include "aliro/utils.h"
#include "external_nvs_ids.h"

#include <external_nvs/external_nvs.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(access_document, CONFIG_DOOR_LOCK_APP_LOG_LEVEL);

namespace Aliro {

namespace {

DoorLock::ExternalNvs::Id GetExternalNvsId(size_t index)
{
	return DoorLock::Storage::ExternalNvsIds::kAccessDocumentRangeStart +
	       static_cast<DoorLock::ExternalNvs::Id>(index);
}

bool IsIndexInRange(size_t index)
{
	return index < DoorLock::Storage::ExternalNvsIds::kAccessDocumentRangeSize;
}

int ReadAccessDocumentHelper(size_t index, AccessDocument &ad)
{
	const auto id = GetExternalNvsId(index);
	size_t len = sizeof(AccessDocument);
	const auto error = DoorLock::ExternalNvs::Read(id, &ad, len);

	if (error != 0) {
		return error;
	}

	if (len != sizeof(AccessDocument)) {
		LOG_ERR("Invalid Access Document size at index: %zu, expected: %zu, got: %zu", index,
			sizeof(AccessDocument), len);
		return -EIO;
	}

	return 0;
}

} // namespace

AliroError LoadAccessDocuments()
{
	AccessDocument ad;
	for (size_t index = 0; index < CONFIG_DOOR_LOCK_STORAGE_MAX_STORED_ACCESS_DOCUMENTS; index++) {
		const auto error = ReadAccessDocumentHelper(index, ad);
		if (error == -ENOENT) {
			continue;
		}

		VerifyOrReturnStatus(error == 0, AliroError::FromInt(error),
				     LOG_ERR("Failed to read Access Document at index: %zu, error code: %d", index,
					     error));

		ReturnErrorOnFailure(AccessManagerInstance().AddPublicKey(
			ad.mPublicKey, AccessManager::PublicKeyType::AccessDocument, index));

		LOG_DBG("Loaded AD at index: %zu, Version: %u, CI index: %u, Timestamp: %.*s, Access Iteration: %" PRIu64,
			index, ad.mVersion, ad.mCredentialIssuerKeyIndex, ad.mSignedTimestamp.size(),
			ad.mSignedTimestamp.data(), ad.mAccessIteration);
	}

	return ALIRO_NO_ERROR;
}

AliroError StoreAccessDocument(size_t index, const AccessDocument &ad)
{
	VerifyOrReturnStatus(IsIndexInRange(index), ALIRO_INVALID_ARGUMENT,
			     LOG_ERR("Access Document index out of range: %zu", index));

	const auto id = GetExternalNvsId(index);
	const auto error = DoorLock::ExternalNvs::Write(id, &ad, sizeof(AccessDocument));
	VerifyOrReturnStatus(error == 0, AliroError::FromInt(error),
			     LOG_ERR("Failed to store Access Document at index: %zu", index));

	return ALIRO_NO_ERROR;
}

AliroError ReadAccessDocument(size_t index, AccessDocument &ad)
{
	VerifyOrReturnStatus(IsIndexInRange(index), ALIRO_INVALID_ARGUMENT,
			     LOG_ERR("Access Document index out of range: %zu", index));

	const auto error = ReadAccessDocumentHelper(index, ad);
	VerifyOrReturnStatus(error == 0, AliroError::FromInt(error),
			     LOG_ERR("Failed to get Access Document at index: %zu", index));

	return ALIRO_NO_ERROR;
}

AliroError ClearAccessDocument(size_t index)
{
	VerifyOrReturnStatus(IsIndexInRange(index), ALIRO_INVALID_ARGUMENT,
			     LOG_ERR("Access Document index out of range: %zu", index));

	const auto id = GetExternalNvsId(index);
	const auto error = DoorLock::ExternalNvs::Delete(id);
	VerifyOrReturnStatus(error == 0, AliroError::FromInt(error),
			     LOG_ERR("Failed to clear Access Document at index: %zu", index));

	return ALIRO_NO_ERROR;
}

} // namespace Aliro
