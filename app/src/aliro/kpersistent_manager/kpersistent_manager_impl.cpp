/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "kpersistent_manager_impl.h"

#include "access_manager.h"
#include "aliro/errors.h"
#include "aliro/interface.h"
#include "aliro/utils.h"
#include "psa_key_ids.h"

#include <crypto_utils/crypto_utils.h>

#include "zephyr/sys/util.h"
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(kpersistent_manager, CONFIG_DOOR_LOCK_APP_LOG_LEVEL);

namespace Aliro {

using namespace CryptoTypes;

namespace {

constexpr KeyId ToKpersistentKeyId(size_t keyOffset)
{
	return DoorLock::Storage::PsaKeyIds::kKpersistentRangeBegin + keyOffset;
}

constexpr size_t FromKpersistentKeyId(KeyId keyId)
{
	return keyId - DoorLock::Storage::PsaKeyIds::kKpersistentRangeBegin;
}

bool IsKpersistentKeyIdValid(KeyId keyId)
{
	constexpr KeyId kKpersistentRangeEnd{ DoorLock::Storage::PsaKeyIds::kKpersistentRangeBegin +
					      DoorLock::Storage::PsaKeyIds::kKpersistentRangeSize };
	return IN_RANGE(keyId, DoorLock::Storage::PsaKeyIds::kKpersistentRangeBegin, kKpersistentRangeEnd - 1);
}

} // namespace

void KpersistentManagerImpl::Init()
{
	VerifyOrDie(mKpersistentCount == 0, "Kpersistent manager is already initialized");

	for (size_t i = 0; i < kMaxKpersistentCount; i++) {
		const auto kpersistentKeyId = ToKpersistentKeyId(i);

		auto error = DoorLock::CryptoUtils::IsKeyAvailable(kpersistentKeyId);
		if (error != ALIRO_NO_ERROR) {
			continue;
		}

		LOG_DBG("Found Kpersistent key with ID: 0x%08x, index: %d", kpersistentKeyId, i);
		mKpersistentMap[i] = true;
		mKpersistentCount++;
	}

	LOG_INF("Loaded %d Kpersistent keys", mKpersistentCount);
}

AliroError KpersistentManagerImpl::GetKpersistentCount(size_t &count)
{
	count = mKpersistentCount;
	return ALIRO_NO_ERROR;
}

AliroError KpersistentManagerImpl::GetKpersistentKeyIds(KeyId *keyIds, size_t &count)
{
	size_t foundCount{ 0 };
	for (size_t i = 0; i < kMaxKpersistentCount && foundCount < count; i++) {
		if (mKpersistentMap[i]) {
			keyIds[foundCount++] = ToKpersistentKeyId(i);
		}
	}

	count = foundCount;

	return ALIRO_NO_ERROR;
}

AliroError KpersistentManagerImpl::PreserveKpersistent(const PublicKey &publicKey, KeyId kpersistentKeyId)
{
	size_t index{};
	VerifyOrReturnStatus(AccessManagerInstance().IsPublicKeyStored(publicKey, &index), ALIRO_PUBLIC_KEY_NOT_FOUND,
			     LOG_WRN("No user public key found in Access Manager for a given Kpersistent"));
	VerifyOrReturnStatus(IN_RANGE(index, 0, kMaxKpersistentCount - 1), ALIRO_NO_MEMORY);

	auto kpersistentKeyIdPersistent = ToKpersistentKeyId(index);
	VerifyOrReturnStatus(IsKpersistentKeyIdValid(kpersistentKeyIdPersistent), ALIRO_INVALID_ARGUMENT,
			     LOG_WRN("Kpersistent key ID is out of range"));

	LOG_DBG("Preserving Kpersistent key with ID: 0x%08x", kpersistentKeyIdPersistent);

	if (mKpersistentMap[index]) {
		LOG_DBG("Removing existing Kpersistent key");

		auto tempKpersistentKeyId{ kpersistentKeyIdPersistent };
		auto status = DoorLock::CryptoUtils::DestroyKey(tempKpersistentKeyId);
		VerifyOrReturnStatus(status == ALIRO_NO_ERROR, status, LOG_ERR("Cannot remove existing Kpersistent"));

		mKpersistentMap[index] = false;
		mKpersistentCount--;
	}

	AliroError status = DoorLock::CryptoUtils::PreserveKey(kpersistentKeyId, kpersistentKeyIdPersistent);
	VerifyOrReturnStatus(status == ALIRO_NO_ERROR, status, LOG_ERR("Cannot preserve new Kpersistent"));

	mKpersistentMap[index] = true;
	mKpersistentCount++;

	return ALIRO_NO_ERROR;
}

AliroError KpersistentManagerImpl::RemoveKpersistent(size_t kpersistentKeyOffset)
{
	LOG_DBG("Removing Kpersistent with index: %d", kpersistentKeyOffset);
	VerifyOrReturnStatus(IN_RANGE(kpersistentKeyOffset, 0, kMaxKpersistentCount - 1), ALIRO_INVALID_ARGUMENT);

	if (!mKpersistentMap[kpersistentKeyOffset]) {
		LOG_WRN("Kpersistent with index: %d is not found", kpersistentKeyOffset);
		return ALIRO_INVALID_ARGUMENT;
	}

	auto kPersistentKeyIdPersistent = ToKpersistentKeyId(kpersistentKeyOffset);
	VerifyOrReturnStatus(IsKpersistentKeyIdValid(kPersistentKeyIdPersistent), ALIRO_INVALID_ARGUMENT,
			     LOG_WRN("Kpersistent key ID is out of range"));

	VerifyOrReturnStatus(DoorLock::CryptoUtils::DestroyKey(kPersistentKeyIdPersistent) == ALIRO_NO_ERROR,
			     ALIRO_ERROR_INTERNAL,
			     LOG_WRN("Cannot remove Kpersistent with key ID: 0x%08x", kPersistentKeyIdPersistent));

	mKpersistentMap[kpersistentKeyOffset] = false;
	mKpersistentCount--;

	return ALIRO_NO_ERROR;
}

void KpersistentManagerImpl::RemoveAllKpersistent()
{
	for (size_t i = 0; i < kMaxKpersistentCount; i++) {
		if (mKpersistentMap[i]) {
			RemoveKpersistent(i);
		}
	}
}

AliroError KpersistentManagerImpl::GetAccessCredentialPublicKey(CryptoTypes::KeyId kpersistentKeyId,
								CryptoTypes::PublicKey &publicKey)
{
	VerifyOrReturnStatus(IsKpersistentKeyIdValid(kpersistentKeyId), ALIRO_INVALID_ARGUMENT);

	// Keys are stored linearly, so we can get the access credential public key id by subtracting the kpersistent
	// key id from the kpersistent begin key id. Plus, it is ensured that for a given kpersistent key id, the
	// access credential public key id is always available (it doesn't work the other way around).
	auto index = FromKpersistentKeyId(kpersistentKeyId);
	VerifyOrReturnStatus(IN_RANGE(index, 0, kMaxKpersistentCount - 1), ALIRO_INVALID_ARGUMENT);
	VerifyOrReturnStatus(mKpersistentMap[index], ALIRO_INVALID_ARGUMENT);

	AliroError status = AccessManagerInstance().GetPublicKey(index, publicKey);
	VerifyOrReturnStatus(status == ALIRO_NO_ERROR, ALIRO_PUBLIC_KEY_NOT_FOUND,
			     LOG_ERR("Cannot get Access Credential public key"));

	return ALIRO_NO_ERROR;
}

} // namespace Aliro
