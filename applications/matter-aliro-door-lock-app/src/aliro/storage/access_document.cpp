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

#ifdef CONFIG_DOOR_LOCK_STORAGE_MIGRATE_ACCESS_DOCUMENT_V1
constexpr int kAccessDocumentV1Found{ 1 };

struct AccessDocumentV1 {
	uint32_t mVersion;
	size_t mCredentialIssuerKeyIndex;
	Aliro::Timestamp mSignedTimestamp;
	ValidityIteration mAccessIteration;
	Aliro::CryptoTypes::PublicKey mPublicKey;
	size_t mAccessDocumentSize;
	std::array<uint8_t, AccessDocument::kAccessDocumentSize> mAccessDocument;
};
#endif // CONFIG_DOOR_LOCK_STORAGE_MIGRATE_ACCESS_DOCUMENT_V1

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

#ifdef CONFIG_DOOR_LOCK_STORAGE_MIGRATE_ACCESS_DOCUMENT_V1
	if (len == sizeof(AccessDocumentV1) && ad.mVersion == 1) {
		return kAccessDocumentV1Found;
	}
#endif // CONFIG_DOOR_LOCK_STORAGE_MIGRATE_ACCESS_DOCUMENT_V1

	if (len != sizeof(AccessDocument)) {
		LOG_ERR("Invalid Access Document size at index: %zu, expected: %zu, got: %zu", index,
			sizeof(AccessDocument), len);
		return -EIO;
	}

	if (ad.mVersion != AccessDocument::kVersion) {
		LOG_ERR("Unsupported Access Document version at index %zu: %u", index, ad.mVersion);
		return -EIO;
	}

	return 0;
}

} // namespace

AliroError UpdateAliroEvictableCredential(size_t index, const CryptoTypes::PublicKey &publicKey,
					  size_t credentialIssuerKeyIndex);

AliroError RemoveAliroEvictableCredential(size_t index, bool updateUser);

AliroError LoadAccessDocuments()
{
	AccessDocument ad;
	for (size_t index = 0; index < CONFIG_DOOR_LOCK_STORAGE_MAX_STORED_ACCESS_DOCUMENTS; index++) {
		const auto error = ReadAccessDocumentHelper(index, ad);
		if (error == -ENOENT) {
			continue;
		}

#ifdef CONFIG_DOOR_LOCK_STORAGE_MIGRATE_ACCESS_DOCUMENT_V1
		if (error == kAccessDocumentV1Found) {
			LOG_INF("Removing version 1 Access Document at index: %zu", index);
			ReturnErrorOnFailure(AccessManagerInstance().AddPublicKey(
				CryptoTypes::PublicKey{}, AccessManager::PublicKeyType::AccessDocument, index));
			ReturnErrorOnFailure(AccessManagerInstance().RemovePublicKey(
				AccessManager::PublicKeyType::AccessDocument, index));
			continue;
		}
#endif // CONFIG_DOOR_LOCK_STORAGE_MIGRATE_ACCESS_DOCUMENT_V1

		VerifyOrReturnStatus(error == 0, AliroError::FromInt(error),
				     LOG_ERR("Failed to read Access Document at index: %zu, error code: %d", index,
					     error));

		ReturnErrorOnFailure(AccessManagerInstance().AddPublicKey(
			ad.mPublicKey, AccessManager::PublicKeyType::AccessDocument, index));

		ReturnErrorOnFailure(
			UpdateAliroEvictableCredential(index, ad.mPublicKey, ad.mCredentialIssuerKeyIndex));

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

	ReturnErrorOnFailure(UpdateAliroEvictableCredential(index, ad.mPublicKey, ad.mCredentialIssuerKeyIndex));

	const auto id = GetExternalNvsId(index);
	const auto error = DoorLock::ExternalNvs::Write(id, &ad, sizeof(AccessDocument));
	if (error != 0) {
		LOG_ERR("Failed to store Access Document at index: %zu", index);
		const auto rollbackError = RemoveAliroEvictableCredential(index, true);
		if (rollbackError != ALIRO_NO_ERROR) {
			LOG_ERR("Failed to revert Aliro evictable credential at index: %zu, error code: %d", index,
				rollbackError.ToInt());
		}
		return AliroError::FromInt(error);
	}

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

AliroError ClearAccessDocument(size_t index, bool updateUser)
{
	VerifyOrReturnStatus(IsIndexInRange(index), ALIRO_INVALID_ARGUMENT,
			     LOG_ERR("Access Document index out of range: %zu", index));

	const auto id = GetExternalNvsId(index);
	const auto error = DoorLock::ExternalNvs::Delete(id);
	VerifyOrReturnStatus(error == 0, AliroError::FromInt(error),
			     LOG_ERR("Failed to clear Access Document at index: %zu", index));

	ReturnErrorOnFailure(RemoveAliroEvictableCredential(index, updateUser));

	return ALIRO_NO_ERROR;
}

} // namespace Aliro
