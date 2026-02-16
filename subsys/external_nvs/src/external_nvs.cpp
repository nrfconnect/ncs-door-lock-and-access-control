/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "external_nvs/external_nvs.h"

#include "aead.h"
#include "key.h"
#include "nonce.h"
#include "storage.h"

#ifdef CONFIG_DOOR_LOCK_EXTERNAL_NVS_ROLLBACK_PROTECTION
#include "counter.h"
#endif // CONFIG_DOOR_LOCK_EXTERNAL_NVS_ROLLBACK_PROTECTION

#include <mbedtls/platform_util.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/util.h>

#include <cstdint>
#include <limits>

LOG_MODULE_REGISTER(external_nvs, CONFIG_DOOR_LOCK_EXTERNAL_NVS_LOG_LEVEL);

/** Mutex protecting all external NVS operations and internal shared state. */
static K_MUTEX_DEFINE(sExternalNvsMutex);

namespace DoorLock::ExternalNvs {

namespace {

constexpr int kErrorInvalidUniqueId{ __ELASTERROR + 1 };

constexpr size_t kMaxDataSize{ CONFIG_DOOR_LOCK_EXTERNAL_NVS_MAX_DATA_SIZE };
constexpr size_t kMaxDataSizeWithTag{ ROUND_UP(kMaxDataSize + Aead::kTagSize, Aead::kTagSize) };

static_assert(kMaxDataSize <= std::numeric_limits<uint16_t>::max(),
	      "CONFIG_DOOR_LOCK_EXTERNAL_NVS_MAX_DATA_SIZE is too big");

using Data = std::array<uint8_t, kMaxDataSizeWithTag>;

struct Aad {
	Id mId;
	uint16_t mDataLength;
#ifdef CONFIG_DOOR_LOCK_EXTERNAL_NVS_ROLLBACK_PROTECTION
	Counter::UniqueId mUniqueId;
	Counter::Counter mCounter;
#endif // CONFIG_DOOR_LOCK_EXTERNAL_NVS_ROLLBACK_PROTECTION

	const uint8_t *ToRawBytes() const { return reinterpret_cast<const uint8_t *>(this); }
};

struct Metadata {
	Aad mAad;
	Nonce::Nonce mNonce;
};

struct Object {
	Metadata mMetadata;
	Data mData;
};

bool IsIdValid(Id id)
{
#ifdef CONFIG_DOOR_LOCK_EXTERNAL_NVS_ROLLBACK_PROTECTION
	return id != kUniqueIdReservedId;
#endif // CONFIG_DOOR_LOCK_EXTERNAL_NVS_ROLLBACK_PROTECTION

	return true;
}

#ifdef CONFIG_DOOR_LOCK_EXTERNAL_NVS_ROLLBACK_PROTECTION

int InitializeUniqueId()
{
	Counter::UniqueId internalUniqueId;
	auto ec = Counter::GetUniqueId(internalUniqueId);
	if (ec != 0) {
		LOG_ERR("Failed to get Unique ID: %d", ec);
		return ec;
	}

	Counter::UniqueId storedUniqueId;
	size_t readLength{ storedUniqueId.size() };
	ec = Storage::Read(kUniqueIdReservedId, storedUniqueId.data(), readLength);
	if (ec == -ENOENT) {
		LOG_INF("Unique ID not found on storage, storing current one");
		ec = Storage::Write(kUniqueIdReservedId, internalUniqueId.data(), internalUniqueId.size());
		if (ec != 0) {
			LOG_ERR("Failed to write unique ID: %d", ec);
			return ec;
		}
	} else if (ec != 0) {
		LOG_ERR("Failed to read Unique ID: %d", ec);
		return ec;
	} else if (readLength != storedUniqueId.size()) {
		LOG_ERR("Unique ID length mismatch, expected %zu, got %zu", storedUniqueId.size(), readLength);
		return -kErrorInvalidUniqueId;
	} else if (storedUniqueId != internalUniqueId) {
		LOG_ERR("Unique ID mismatch");
		return -kErrorInvalidUniqueId;
	}

	return 0;
}

#endif // CONFIG_DOOR_LOCK_EXTERNAL_NVS_ROLLBACK_PROTECTION

int Write(Id id, const void *data, size_t len, Object &object, Key::Key &key)
{
	int ec{ -EIO };

	object.mMetadata.mAad.mId = id;
	object.mMetadata.mAad.mDataLength = static_cast<uint16_t>(len);

#ifdef CONFIG_DOOR_LOCK_EXTERNAL_NVS_ROLLBACK_PROTECTION
	ec = Counter::GetUniqueId(object.mMetadata.mAad.mUniqueId);
	if (ec != 0) {
		LOG_ERR("Failed to get unique ID: %d", ec);
		return ec;
	}

	ec = Counter::Read(id, object.mMetadata.mAad.mCounter);
	if (ec != 0) {
		LOG_ERR("Failed to read counter: %d", ec);
		return ec;
	}

	object.mMetadata.mAad.mCounter++;
#endif // CONFIG_DOOR_LOCK_EXTERNAL_NVS_ROLLBACK_PROTECTION

	ec = Nonce::Generate(object.mMetadata.mNonce);
	if (ec != 0) {
		return ec;
	}

	ec = Key::Generate(id, key);
	if (ec != 0) {
		return ec;
	}

	const auto &nonce{ object.mMetadata.mNonce };
	const uint8_t *aad{ reinterpret_cast<const uint8_t *>(&object.mMetadata.mAad) };
	const size_t aadLength{ sizeof(object.mMetadata.mAad) };

	size_t cipherTextLength{ object.mData.size() };
	ec = Aead::Encrypt(key, nonce, aad, aadLength, reinterpret_cast<const uint8_t *>(data), len,
			   object.mData.data(), cipherTextLength);
	if (ec != 0) {
		return ec;
	}

#ifdef CONFIG_DOOR_LOCK_EXTERNAL_NVS_ROLLBACK_PROTECTION
	ec = Counter::Write(id, object.mMetadata.mAad.mCounter);
	if (ec != 0) {
		LOG_ERR("Failed to write counter: %d", ec);
		return ec;
	}
#endif // CONFIG_DOOR_LOCK_EXTERNAL_NVS_ROLLBACK_PROTECTION

	ec = Storage::Write(id, &object, offsetof(Object, mData) + cipherTextLength);
	if (ec != 0) {
		return ec;
	}

	return 0;
}

int Read(Id id, void *data, size_t &len, Object &object, Key::Key &key)
{
	size_t readLength{ sizeof(object) };
	auto ec = Storage::Read(id, &object, readLength);
	if (ec != 0) {
		return ec;
	}

	if (readLength < sizeof(object.mMetadata)) {
		LOG_ERR("Read length is too short: %zu", readLength);
		return -EIO;
	}

	const auto &metadata{ object.mMetadata };

	if (id != metadata.mAad.mId) {
		LOG_ERR("Id mismatch, expected %u, got %u", id, metadata.mAad.mId);
		return -EIO;
	}

#ifdef CONFIG_DOOR_LOCK_EXTERNAL_NVS_ROLLBACK_PROTECTION
	Counter::UniqueId internalUniqueId;
	ec = Counter::GetUniqueId(internalUniqueId);
	if (ec != 0) {
		LOG_ERR("Failed to get unique ID: %d", ec);
		return ec;
	}

	if (internalUniqueId != metadata.mAad.mUniqueId) {
		LOG_ERR("Unique ID mismatch");
		return -EIO;
	}

	Counter::Counter internalCounter;
	ec = Counter::Read(id, internalCounter);
	if (ec != 0) {
		LOG_ERR("Failed to get current counter: %d", ec);
		return ec;
	}

	if (internalCounter != metadata.mAad.mCounter) {
		LOG_ERR("Counter mismatch, expected %u, got %u", internalCounter, metadata.mAad.mCounter);
		return -EIO;
	}
#endif // CONFIG_DOOR_LOCK_EXTERNAL_NVS_ROLLBACK_PROTECTION

	ec = Key::Generate(id, key);
	if (ec != 0) {
		return ec;
	}

	const auto &nonce{ object.mMetadata.mNonce };
	const uint8_t *aad{ object.mMetadata.mAad.ToRawBytes() };
	const size_t aadLength{ sizeof(object.mMetadata.mAad) };
	const uint8_t *cipherText{ object.mData.data() };
	const size_t cipherTextLength{ readLength - offsetof(Object, mData) };

	ec = Aead::Decrypt(key, nonce, aad, aadLength, cipherText, cipherTextLength, reinterpret_cast<uint8_t *>(data),
			   len);
	if (ec != 0) {
		return ec;
	}

	if (len != metadata.mAad.mDataLength) {
		LOG_ERR("Data length mismatch, expected %u, got %zu", metadata.mAad.mDataLength, len);
		return -EIO;
	}

	return 0;
}

int InitLocked(uint8_t partitionId)
{
	auto ec = Storage::Init(partitionId);
	if (ec != 0) {
		return ec;
	}

	ec = Nonce::Init();
	if (ec != 0) {
		return ec;
	}

	ec = Key::Init();
	if (ec != 0) {
		return ec;
	}

#ifdef CONFIG_DOOR_LOCK_EXTERNAL_NVS_ROLLBACK_PROTECTION
	ec = Counter::Init();
	if (ec != 0) {
		return ec;
	}

	ec = InitializeUniqueId();
	if (ec == -kErrorInvalidUniqueId) {
		ec = Storage::Clear();
		if (ec != 0) {
			return ec;
		}

		ec = InitializeUniqueId();
		if (ec != 0) {
			return ec;
		}
	} else if (ec != 0) {
		return ec;
	}
#endif // CONFIG_DOOR_LOCK_EXTERNAL_NVS_ROLLBACK_PROTECTION

	return 0;
}

int ClearLocked()
{
#ifdef CONFIG_DOOR_LOCK_EXTERNAL_NVS_ROLLBACK_PROTECTION
	const auto ec = Counter::Clear();
	if (ec != 0) {
		return ec;
	}
#endif // CONFIG_DOOR_LOCK_EXTERNAL_NVS_ROLLBACK_PROTECTION

	return Storage::Clear();
}

int WriteLocked(Id id, const void *data, size_t len)
{
	Object object;
	Key::Key key;

	const auto ec = Write(id, data, len, object, key);

	mbedtls_platform_zeroize(&object, sizeof(object));
	mbedtls_platform_zeroize(&key, sizeof(key));

	if (ec != 0) {
		LOG_ERR("Failed to write object at Id %u: %d", id, ec);
		Storage::Delete(id);
	}

	return ec;
}

int ReadLocked(Id id, void *data, size_t &len)
{
	Object object;
	Key::Key key;

	const auto ec = Read(id, data, len, object, key);

	mbedtls_platform_zeroize(&object, sizeof(object));
	mbedtls_platform_zeroize(&key, sizeof(key));

	if (ec != 0) {
		LOG_ERR("Failed to read object at Id %u: %d", id, ec);

		if (ec == -EIO) {
			Storage::Delete(id);
		}
	}

	return ec;
}

int DeleteLocked(Id id)
{
	int ec{ -EIO };

#ifdef CONFIG_DOOR_LOCK_EXTERNAL_NVS_ROLLBACK_PROTECTION
	Counter::Counter counter;
	ec = Counter::Read(id, counter);
	if (ec != 0) {
		LOG_ERR("Failed to read counter: %d", ec);
		return ec;
	}

	counter++;
	ec = Counter::Write(id, counter);
	if (ec != 0) {
		LOG_ERR("Failed to write counter: %d", ec);
		return ec;
	}
#endif // CONFIG_DOOR_LOCK_EXTERNAL_NVS_ROLLBACK_PROTECTION

	ec = Storage::Delete(id);
	if (ec != 0) {
		return ec;
	}

	return 0;
}

} // namespace

int Init(uint8_t partitionId)
{
	k_mutex_lock(&sExternalNvsMutex, K_FOREVER);
	const auto ec = InitLocked(partitionId);
	k_mutex_unlock(&sExternalNvsMutex);
	return ec;
}

int Clear()
{
	k_mutex_lock(&sExternalNvsMutex, K_FOREVER);
	const auto ec = ClearLocked();
	k_mutex_unlock(&sExternalNvsMutex);
	return ec;
}

int Write(Id id, const void *data, size_t len)
{
	if (!IsIdValid(id)) {
		LOG_ERR("Invalid ID: %u", id);
		return -EINVAL;
	}

	if (data == nullptr) {
		LOG_ERR("data is null");
		return -EINVAL;
	}

	if (len > kMaxDataSize) {
		LOG_ERR("length is too long: %zu", len);
		return -EINVAL;
	}

	k_mutex_lock(&sExternalNvsMutex, K_FOREVER);
	const auto ec = WriteLocked(id, data, len);
	k_mutex_unlock(&sExternalNvsMutex);
	return ec;
}

int Read(Id id, void *data, size_t &len)
{
	if (!IsIdValid(id)) {
		LOG_ERR("Invalid ID: %u", id);
		return -EINVAL;
	}

	if (data == nullptr) {
		LOG_ERR("data is null");
		return -EINVAL;
	}

	k_mutex_lock(&sExternalNvsMutex, K_FOREVER);
	const auto ec = ReadLocked(id, data, len);
	k_mutex_unlock(&sExternalNvsMutex);
	return ec;
}

int Delete(Id id)
{
	if (!IsIdValid(id)) {
		LOG_ERR("Invalid ID: %u", id);
		return -EINVAL;
	}

	k_mutex_lock(&sExternalNvsMutex, K_FOREVER);
	const auto ec = DeleteLocked(id);
	k_mutex_unlock(&sExternalNvsMutex);
	return ec;
}

} // namespace DoorLock::ExternalNvs
