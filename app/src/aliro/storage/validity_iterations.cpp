/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "validity_iterations.h"

#include "aliro/utils.h"
#include "storage.h"
#include "storage_keys.h"

#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(validity_iterations, CONFIG_DOOR_LOCK_APP_LOG_LEVEL);

namespace Aliro {

AliroError ReadValidityIterations(size_t credentialIssuerKeyIndex, ValidityIterations &iterations)
{
	const auto keyName = KeyValueStorage::GetStorageKeyName(
		StorageKeys::kStorageKeyNameCredentialIssuerValidityIteration, credentialIssuerKeyIndex);
	const auto status = KeyValueStorage::Instance().Get(keyName.data(), reinterpret_cast<uint8_t *>(&iterations),
							    sizeof(ValidityIterations));
	VerifyOrReturnStatus(status == 0 || status == -ENOENT, ALIRO_ERROR_INTERNAL,
			     LOG_ERR("Cannot get ValidityIterations from persistent storage"));

	if (status == -ENOENT) {
		iterations = ValidityIterations{};
	}

	return ALIRO_NO_ERROR;
}

AliroError StoreValidityIterations(size_t credentialIssuerKeyIndex, const ValidityIterations &iterations)
{
	const auto keyName = KeyValueStorage::GetStorageKeyName(
		StorageKeys::kStorageKeyNameCredentialIssuerValidityIteration, credentialIssuerKeyIndex);
	VerifyOrReturnStatus(!KeyValueStorage::Instance().Save(keyName.data(),
							       reinterpret_cast<const uint8_t *>(&iterations),
							       sizeof(ValidityIterations)),
			     ALIRO_ERROR_INTERNAL, LOG_ERR("Cannot save ValidityIterations to persistent storage"));

	return ALIRO_NO_ERROR;
}

AliroError ClearValidityIterations(size_t credentialIssuerKeyIndex)
{
	const auto keyName = KeyValueStorage::GetStorageKeyName(
		StorageKeys::kStorageKeyNameCredentialIssuerValidityIteration, credentialIssuerKeyIndex);
	VerifyOrReturnStatus(KeyValueStorage::Instance().Clear(keyName.data()) == 0, ALIRO_ERROR_INTERNAL,
			     LOG_ERR("Cannot clear Validity Iterations from persistent storage"));

	return ALIRO_NO_ERROR;
}

} // namespace Aliro
