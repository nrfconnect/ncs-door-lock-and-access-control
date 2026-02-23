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

constexpr size_t kMaxDataSize{ CONFIG_DOOR_LOCK_EXTERNAL_NVS_MAX_DATA_SIZE };

static_assert(kMaxDataSize <= std::numeric_limits<uint16_t>::max(),
	      "CONFIG_DOOR_LOCK_EXTERNAL_NVS_MAX_DATA_SIZE is too big");

using Ciphertext = Aead::Ciphertext<kMaxDataSize>;

struct Aad {
	Id mId;
#ifdef CONFIG_DOOR_LOCK_EXTERNAL_NVS_ROLLBACK_PROTECTION
	Counter::UniqueId mUniqueId;
	Counter::Counter mCounter;
#endif // CONFIG_DOOR_LOCK_EXTERNAL_NVS_ROLLBACK_PROTECTION

	/* Zeroize the AAD on destruction. */
	~Aad() { mbedtls_platform_zeroize(this, sizeof(*this)); }

	/* Delete copy and move constructors and operators. */
	Aad(const Aad &) = delete;
	Aad &operator=(const Aad &) = delete;
	Aad(Aad &&) = delete;
	Aad &operator=(Aad &&) = delete;

	const uint8_t *ToRawBytes() const { return reinterpret_cast<const uint8_t *>(this); }
	constexpr size_t Size() const { return sizeof(*this); }
} __packed;

struct Metadata {
	Nonce::Nonce mNonce;
#ifdef CONFIG_DOOR_LOCK_EXTERNAL_NVS_ROLLBACK_PROTECTION
	Counter::Counter mCounter;
#endif // CONFIG_DOOR_LOCK_EXTERNAL_NVS_ROLLBACK_PROTECTION
};

struct Object {
	Metadata mMetadata;
	Ciphertext mCiphertext;
};

bool IsIdValid(Id id)
{
#ifdef CONFIG_DOOR_LOCK_EXTERNAL_NVS_ROLLBACK_PROTECTION
	return id != kUniqueIdReservedId;
#endif // CONFIG_DOOR_LOCK_EXTERNAL_NVS_ROLLBACK_PROTECTION

	return true;
}

#ifdef CONFIG_DOOR_LOCK_EXTERNAL_NVS_ROLLBACK_PROTECTION

int InitializeUniqueId(uint8_t partitionId)
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

	if (ec == 0 && readLength == internalUniqueId.size() && storedUniqueId == internalUniqueId) {
		return 0;
	}

#ifdef CONFIG_LOG
	if (ec == -ENOENT) {
		LOG_INF("Unique ID not found on storage");
	} else if (ec != 0) {
		LOG_ERR("Failed to read Unique ID: %d", ec);
	} else if (readLength != storedUniqueId.size()) {
		LOG_ERR("Unique ID length mismatch, expected %zu, got %zu", storedUniqueId.size(), readLength);
	} else if (storedUniqueId != internalUniqueId) {
		LOG_ERR("Unique ID mismatch");
	}
#endif // CONFIG_LOG

	LOG_INF("Clearing the storage");
	ec = Storage::Clear();
	if (ec != 0) {
		return ec;
	}

	LOG_INF("Reinitializing the storage");
	ec = Storage::Init(partitionId);
	if (ec != 0) {
		return ec;
	}

	LOG_INF("Writing new unique ID to storage");
	ec = Storage::Write(kUniqueIdReservedId, internalUniqueId.data(), internalUniqueId.size());
	if (ec != 0) {
		LOG_ERR("Failed to write unique ID: %d", ec);
		return ec;
	}

	return 0;
}

#endif // CONFIG_DOOR_LOCK_EXTERNAL_NVS_ROLLBACK_PROTECTION

int Write(Id id, const void *data, size_t len)
{
	int ec{ -EIO };
	Object object;

	Aad aad{};
	aad.mId = id;

#ifdef CONFIG_DOOR_LOCK_EXTERNAL_NVS_ROLLBACK_PROTECTION
	ec = Counter::GetUniqueId(aad.mUniqueId);
	if (ec != 0) {
		LOG_ERR("Failed to get unique ID: %d", ec);
		return ec;
	}

	Counter::Counter counter;
	ec = Counter::Read(id, counter);
	if (ec != 0) {
		LOG_ERR("Failed to read counter: %d", ec);
		return ec;
	}

	counter++;

	aad.mCounter = counter;
	object.mMetadata.mCounter = counter;
#endif // CONFIG_DOOR_LOCK_EXTERNAL_NVS_ROLLBACK_PROTECTION

	ec = Nonce::Generate(object.mMetadata.mNonce);
	if (ec != 0) {
		return ec;
	}

	Key::Key key;
	ec = Key::Derive(id, key);
	if (ec != 0) {
		return ec;
	}

	const auto &nonce{ object.mMetadata.mNonce };
	size_t cipherTextLength{ object.mCiphertext.size() };

	ec = Aead::Encrypt(key, nonce, aad.ToRawBytes(), aad.Size(), reinterpret_cast<const uint8_t *>(data), len,
			   object.mCiphertext.data(), cipherTextLength);
	mbedtls_platform_zeroize(&key, sizeof(key));
	if (ec != 0) {
		return ec;
	}

	const auto writeLength{ offsetof(Object, mCiphertext) + cipherTextLength };
	ec = Storage::Write(id, &object, writeLength);
	if (ec != 0) {
		return ec;
	}

#ifdef CONFIG_DOOR_LOCK_EXTERNAL_NVS_ROLLBACK_PROTECTION
	ec = Counter::Write(id, object.mMetadata.mCounter);
	if (ec != 0) {
		LOG_ERR("Failed to write counter: %d", ec);
		return ec;
	}
#endif // CONFIG_DOOR_LOCK_EXTERNAL_NVS_ROLLBACK_PROTECTION

	return 0;
}

int Read(Id id, void *data, size_t &len)
{
	Object object;
	size_t readLength{ sizeof(object) };
	auto ec = Storage::Read(id, &object, readLength);
	if (ec != 0) {
		return ec;
	}

	if (readLength < sizeof(object.mMetadata)) {
		LOG_ERR("Read length is too short: %zu", readLength);
		return -EIO;
	}

	Aad aad{};
	aad.mId = id;

#ifdef CONFIG_DOOR_LOCK_EXTERNAL_NVS_ROLLBACK_PROTECTION
	ec = Counter::GetUniqueId(aad.mUniqueId);
	if (ec != 0) {
		LOG_ERR("Failed to get unique ID: %d", ec);
		return ec;
	}

	Counter::Counter counter;
	ec = Counter::Read(id, counter);
	if (ec != 0) {
		LOG_ERR("Failed to get current counter: %d", ec);
		return ec;
	}

	aad.mCounter = counter;
#endif // CONFIG_DOOR_LOCK_EXTERNAL_NVS_ROLLBACK_PROTECTION

	Key::Key key;
	ec = Key::Derive(id, key);
	if (ec != 0) {
		return ec;
	}

	const auto &nonce{ object.mMetadata.mNonce };
	const uint8_t *cipherText{ object.mCiphertext.data() };
	const size_t cipherTextLength{ readLength - offsetof(Object, mCiphertext) };

	ec = Aead::Decrypt(key, nonce, aad.ToRawBytes(), aad.Size(), cipherText, cipherTextLength,
			   reinterpret_cast<uint8_t *>(data), len);
	mbedtls_platform_zeroize(&key, sizeof(key));
	if (ec != 0) {
		return ec;
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

	ec = InitializeUniqueId(partitionId);
	if (ec != 0) {
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
	const auto ec = Write(id, data, len);
	if (ec != 0) {
		LOG_ERR("Failed to write object at Id %u: %d", id, ec);
		Storage::Delete(id);
	}

	return ec;
}

int ReadLocked(Id id, void *data, size_t &len)
{
	const auto ec = Read(id, data, len);
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
#endif // CONFIG_DOOR_LOCK_EXTERNAL_NVS_ROLLBACK_PROTECTION

	ec = Storage::Delete(id);
	if (ec != 0) {
		return ec;
	}

#ifdef CONFIG_DOOR_LOCK_EXTERNAL_NVS_ROLLBACK_PROTECTION
	counter++;
	ec = Counter::Write(id, counter);
	if (ec != 0) {
		LOG_ERR("Failed to write counter: %d", ec);
		return ec;
	}
#endif // CONFIG_DOOR_LOCK_EXTERNAL_NVS_ROLLBACK_PROTECTION

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
