/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include <array>
#include <cstddef>

namespace Aliro::StorageKeys {

constexpr size_t kKeyNameMaxLength{ 16 };

using KeyNameBuffer = std::array<char, Aliro::StorageKeys::kKeyNameMaxLength>;

constexpr char kDoorLockBaseKey[] = "dl";

#ifndef CONFIG_CHIP

constexpr char kStorageKeyNameAccessCredentialPublicKey[] = "AC_key";
constexpr char kStorageKeyNameCredentialIssuerPublicKey[] = "CI_key";

#endif // CONFIG_CHIP

#ifdef CONFIG_DOOR_LOCK_READER_CERTIFICATE

constexpr size_t kMaxCertificateSize{ CONFIG_DOOR_LOCK_READER_CERTIFICATE_MAX_SIZE };
constexpr char kStorageKeyNameReaderCertificate[] = "r_cert";
constexpr char kStorageKeyNameReaderCertificateLength[] = "r_cert_len";
constexpr char kStorageKeyNameReaderSystemIssuerCAPublicKey[] = "issuer_pk";

#endif // CONFIG_DOOR_LOCK_READER_CERTIFICATE

#ifdef CONFIG_DOOR_LOCK_STEP_UP_PHASE

constexpr char kStorageKeyNameCredentialIssuerValidityIteration[] = "VI";

#endif // CONFIG_DOOR_LOCK_STEP_UP_PHASE

} // namespace Aliro::StorageKeys
