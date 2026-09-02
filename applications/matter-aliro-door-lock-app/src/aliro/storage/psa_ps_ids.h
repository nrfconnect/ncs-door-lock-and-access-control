/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include <psa/protected_storage.h>
#include <zephyr/sys/util.h>

#include <cstdint>

namespace DoorLock::Storage::PsaPsIds {

constexpr psa_storage_uid_t kStorageUidBase{ CONFIG_DOOR_LOCK_ALIRO_PSA_PROTECTED_STORAGE_UID_BASE };
constexpr uint64_t kStorageUidSize{ 0x100000000 }; /* 4GB */

static_assert(IN_RANGE(CONFIG_DOOR_LOCK_ALIRO_READER_STORAGE_IDENTIFIER_PSA_STORAGE_UID, kStorageUidBase,
		       kStorageUidBase + kStorageUidSize - 1),
	      "Reader Identifier PSA Storage UID is out of Aliro PSA Protected Storage UID range");

#ifdef CONFIG_DOOR_LOCK_ACCESS_MANAGER_CREDENTIAL_ISSUER_CERTIFICATE_KEYS

constexpr psa_storage_uid_t kCertificateCredentialIssuerUidBegin{ kStorageUidBase + 0x1000 };
constexpr uint64_t kCertificateCredentialIssuerUidSize{
	CONFIG_DOOR_LOCK_ACCESS_MANAGER_CREDENTIAL_ISSUER_CERTIFICATE_MAX_STORED_KEYS
};

static_assert(kCertificateCredentialIssuerUidBegin + kCertificateCredentialIssuerUidSize - 1 <
		      kStorageUidBase + kStorageUidSize,
	      "Certificate Credential Issuer PSA Storage UIDs are out of Aliro PSA Protected Storage UID range");

#endif // CONFIG_DOOR_LOCK_ACCESS_MANAGER_CREDENTIAL_ISSUER_CERTIFICATE_KEYS

} // namespace DoorLock::Storage::PsaPsIds
