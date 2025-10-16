/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "kpersistent_manager_impl.h"

#include "access_manager/access_manager.h"
#include "aliro/crypto_key_ids.h"
#include "aliro/errors.h"
#include "aliro/utils.h"
#include "crypto/crypto.h"

#include "zephyr/sys/util.h"
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(kpersistent_manager, CONFIG_NCS_DOOR_LOCK_APP_LOG_LEVEL);

namespace Aliro {

using namespace CryptoTypes;

namespace {

constexpr KeyId ToKpersistentKeyId(size_t keyOffset)
{
	return kKpersistentRangeBegin + keyOffset;
}

constexpr size_t FromKpersistentKeyId(KeyId keyId)
{
	return keyId - kKpersistentRangeBegin;
}

} // namespace

AliroError KpersistentManagerImpl::GetKpersistentCount(size_t &count)
{
	count = mKpersistentCount;
	return ALIRO_NO_ERROR;
}

AliroError KpersistentManagerImpl::GetKpersistentKeyIds(KeyId *keyIds, size_t &count)
{
	for (size_t i = 0; i < std::min(count, mKpersistentCount); i++) {
		keyIds[i] = ToKpersistentKeyId(i);
	}
	count = std::min(count, mKpersistentCount);
	return ALIRO_NO_ERROR;
}

AliroError KpersistentManagerImpl::PreserveKpersistent(const PublicKey &publicKey, KeyId &kpersistentKeyId)
{
	VerifyOrReturnStatus(mKpersistentCount < CONFIG_MAX_NUMBER_OF_KPERSISTENT, ALIRO_NO_MEMORY);

	size_t userPublicKeyKeyIdIndex{};
	VerifyOrReturnStatus(AccessManagerInstance().IsPublicKeyStored(publicKey, &userPublicKeyKeyIdIndex),
			     ALIRO_PUBLIC_KEY_NOT_FOUND,
			     LOG_WRN("No user public key found in Access Manager for a given Kpersistent"));

	auto kpersistentKeyIdPersistent = ToKpersistentKeyId(userPublicKeyKeyIdIndex);
	VerifyOrReturnStatus(IN_RANGE(kpersistentKeyIdPersistent, kKpersistentRangeBegin, kKpersistentRangeEnd),
			     ALIRO_INVALID_ARGUMENT, LOG_WRN("Kpersistent key ID is out of range"));

	LOG_DBG("Preserving Kpersistent key with ID: %d", kpersistentKeyIdPersistent);
	AliroError status = CryptoInstance().PreserveKey(kpersistentKeyId, kpersistentKeyIdPersistent);
	if (status == ALIRO_KEY_ALREADY_EXISTS) {
		LOG_DBG("Overwriting an existing Kpersistent key");

		auto tempKpersistentKeyId{ kpersistentKeyIdPersistent };
		status = CryptoInstance().DestroyKey(tempKpersistentKeyId);
		VerifyOrReturnStatus(status == ALIRO_NO_ERROR, status, LOG_ERR("Cannot destroy existing Kpersistent"));

		status = CryptoInstance().PreserveKey(kpersistentKeyId, kpersistentKeyIdPersistent);
		VerifyOrReturnStatus(status == ALIRO_NO_ERROR, status, LOG_ERR("Cannot preserve Kpersistent"));
		kpersistentKeyId = kpersistentKeyIdPersistent;
		return status;
	}
	VerifyOrReturnStatus(status == ALIRO_NO_ERROR, status, LOG_ERR("Cannot preserve new Kpersistent"));

	kpersistentKeyId = kpersistentKeyIdPersistent;
	mKpersistentCount++;

	return ALIRO_NO_ERROR;
}

AliroError KpersistentManagerImpl::RemoveKpersistent(size_t kpersistentKeyOffset)
{
	LOG_DBG("Removing Kpersistent with index: %d", kpersistentKeyOffset);

	auto kPersistentKeyIdPersistent = ToKpersistentKeyId(kpersistentKeyOffset);
	VerifyOrReturnStatus(IN_RANGE(kPersistentKeyIdPersistent, kKpersistentRangeBegin, kKpersistentRangeEnd),
			     ALIRO_INVALID_ARGUMENT, LOG_WRN("Kpersistent key ID is out of range"));

	VerifyOrReturnStatus(CryptoInstance().DestroyKey(kPersistentKeyIdPersistent) == ALIRO_NO_ERROR,
			     ALIRO_ERROR_INTERNAL,
			     LOG_WRN("Cannot destroy Kpersistent with key ID: %d", kPersistentKeyIdPersistent));

	mKpersistentCount--;

	return ALIRO_NO_ERROR;
}

void KpersistentManagerImpl::RemoveAllKpersistent()
{
	for (size_t i = 0; i < mKpersistentCount; i++) {
		RemoveKpersistent(i);
	}
}

AliroError KpersistentManagerImpl::GetAccessCredentialPublicKey(CryptoTypes::KeyId kpersistentKeyId,
								CryptoTypes::PublicKey &publicKey)
{
	// Keys are stored linearly, so we can get the access credential public key id by subtracting the kpersistent
	// key id from the kpersistent begin key id. Plus, it is ensured that for a given kpersistent key id, the
	// access credential public key id is always available (it doesn't work the other way around).
	VerifyOrReturnStatus(mKpersistentCount > 0, ALIRO_INVALID_ARGUMENT);
	VerifyOrReturnStatus(IN_RANGE(kpersistentKeyId, kKpersistentRangeBegin, kKpersistentRangeEnd),
			     ALIRO_INVALID_ARGUMENT);
	auto pubKeyIdIdx = FromKpersistentKeyId(kpersistentKeyId);
	VerifyOrReturnStatus(IN_RANGE(pubKeyIdIdx, 0, mKpersistentCount - 1), ALIRO_INVALID_ARGUMENT);

	AliroError status = AccessManagerInstance().GetPublicKey(pubKeyIdIdx, publicKey);
	VerifyOrReturnStatus(status == ALIRO_NO_ERROR, status, LOG_ERR("Cannot get Access Credential public key"));

	return ALIRO_NO_ERROR;
}

} // namespace Aliro
