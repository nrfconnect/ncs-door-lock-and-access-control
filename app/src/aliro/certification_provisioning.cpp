/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "certification_provisioning.h"

#include "crypto/utils.h"
#include "psa_key_ids.h"
#include "reader.h"
#include "storage.h"
#include "storage_keys.h"

#include <aliro/types.h>
#include <aliro/utils.h>

#include <zephyr/logging/log.h>
#include <zephyr/sys/util.h>

#include <array>
#include <cstring>

LOG_MODULE_REGISTER(certification_provisioning, CONFIG_DOOR_LOCK_APP_LOG_LEVEL);

namespace DoorLock::CertificationProvisioning {
namespace {

using namespace Aliro;

constexpr char kAccessCredentialPublicKeyHex[] =
	"04742df736d0fc9be978c45b00e8fdf7cea684ea105ae574c1505a2c24ab6198e3125b7f1b7e1d134c55ece69681ba"
	"8ecc18a3836dc5199c759f31e8ccf17e3efa";
constexpr char kCredentialIssuerPublicKeyHex[] =
	"047BA31938492E3F5E97BC91806B5835B5D9E426609139006711E5FB7A670EE4E12FC9F25396C013CC20166029D761"
	"A105DEA5E071E84A9E499920524CE2301137";
constexpr char kReaderPrivateKeyHex[] = "8aefdff8d5b47aa9a3edbac7a345ed2221021512fd55abde3b8ee0f208952693";
constexpr char kReaderPublicKeyHex[] =
	"043928f322019d4757893bde6a0fe5e13e3e537b9ca0f549c0bd2f40f79060252a0a4f291192157a95cb6eb202759"
	"428c00cd834998c5d0eab192ee8873c5d34ee";
constexpr char kReaderIdentifierHex[] = "00113344667799AA00113344667799AB113344667799AA00113344667799AA00";
constexpr char kReaderIssuerPublicKeyHex[] =
	"043928f322019d4757893bde6a0fe5e13e3e537b9ca0f549c0bd2f40f79060252a0a4f291192157a95cb6eb202759"
	"428c00cd834998c5d0eab192ee8873c5d34ee";
constexpr char kReaderCertificateHex[] =
	"3081950402000030818e854200043928f322019d4757893bde6a0fe5e13e3e537b9ca0f549c0bd2f40f79060252"
	"a0a4f291192157a95cb6eb202759428c00cd834998c5d0eab192ee8873c5d34ee8648003045022100f509f4e64b31"
	"b5c8d4152158065b4eedd31c66d6e7b1f87975f837f5a3fe1235022063ee11a312731c4673382c7fcde1014407"
	"67ff56654bf64595be802ec0ace3e1";
constexpr char kGroupResolvingKeyHex[] = "00000000000000000000000000000000";

template <typename TByteArray> AliroError DecodeHex(const char *hexString, TByteArray &output)
{
	const size_t hexLength = strlen(hexString);
	VerifyOrReturnStatus(hexLength == output.size() * 2, ALIRO_INVALID_ARGUMENT,
			     LOG_ERR("Invalid test parameter length"));

	const size_t decoded = hex2bin(hexString, hexLength, output.data(), output.size());
	VerifyOrReturnStatus(decoded == output.size(), ALIRO_INVALID_ARGUMENT,
			     LOG_ERR("Invalid test parameter hex"));

	return ALIRO_NO_ERROR;
}

AliroError SavePublicKey(KeyValueStorage::KeyIdString storageKey, size_t keyIndex, const char *publicKeyHex)
{
	CryptoTypes::PublicKey publicKey{};
	ReturnErrorOnFailure(DecodeHex(publicKeyHex, publicKey));

	const auto keyName = KeyValueStorage::GetStorageKeyName(storageKey, keyIndex);
	const int ec = KeyValueStorage::Instance().Save(keyName.data(), publicKey.data(), publicKey.size());
	VerifyOrReturnStatus(ec == 0, ALIRO_ERROR_INTERNAL, LOG_ERR("Failed to save %s/%u", storageKey, keyIndex));

	return ALIRO_NO_ERROR;
}

AliroError EnsureReaderPrivateKey()
{
	CryptoTypes::PrivateKey privateKey{};
	ReturnErrorOnFailure(DecodeHex(kReaderPrivateKeyHex, privateKey));

	CryptoTypes::PublicKey expectedPublicKey{};
	ReturnErrorOnFailure(DecodeHex(kReaderPublicKeyHex, expectedPublicKey));

	if (Storage::Reader::IsPrivateKeySet()) {
		CryptoTypes::PublicKey currentPublicKey{};
		const auto err = Crypto::ExportPublicKey(Storage::PsaKeyIds::kPrivateKeyId, currentPublicKey);
		VerifyOrReturnStatus(err == ALIRO_NO_ERROR, err, LOG_ERR("Failed to export current Reader public key"));

		if (currentPublicKey == expectedPublicKey) {
			return ALIRO_NO_ERROR;
		}

		ReturnErrorOnFailure(Storage::Reader::ClearPrivateKey());
	}

	return Storage::Reader::SetPrivateKey(privateKey);
}

AliroError EnsureReaderIdentifier()
{
	Identifier identifier{};
	ReturnErrorOnFailure(DecodeHex(kReaderIdentifierHex, identifier));

	return Storage::Reader::SetIdentifier(identifier);
}

#ifdef CONFIG_DOOR_LOCK_READER_CERTIFICATE

AliroError EnsureReaderCertificate()
{
	static_assert((sizeof(kReaderCertificateHex) - 1) % 2 == 0, "Certificate hex must have an even length");
	constexpr size_t kReaderCertificateLength = (sizeof(kReaderCertificateHex) - 1) / 2;

	std::array<uint8_t, kReaderCertificateLength> certificate{};
	ReturnErrorOnFailure(DecodeHex(kReaderCertificateHex, certificate));

	return Storage::Reader::SetCertificate(certificate.data(), certificate.size());
}

AliroError EnsureReaderIssuerPublicKey()
{
	CryptoTypes::PublicKey publicKey{};
	ReturnErrorOnFailure(DecodeHex(kReaderIssuerPublicKeyHex, publicKey));

	return Storage::Reader::SetIssuerPublicKey(publicKey);
}

#endif // CONFIG_DOOR_LOCK_READER_CERTIFICATE

#ifdef CONFIG_DOOR_LOCK_BLE_UWB

AliroError EnsureGroupResolvingKey()
{
	CryptoTypes::GroupResolvingKey groupResolvingKey{};
	ReturnErrorOnFailure(DecodeHex(kGroupResolvingKeyHex, groupResolvingKey));

	if (Storage::Reader::IsGroupResolvingKeySet()) {
		CryptoTypes::GroupResolvingKey currentGroupResolvingKey{};
		ReturnErrorOnFailure(Storage::Reader::GetGroupResolvingKey(currentGroupResolvingKey));

		if (currentGroupResolvingKey == groupResolvingKey) {
			return ALIRO_NO_ERROR;
		}

		ReturnErrorOnFailure(Storage::Reader::ClearGroupResolvingKey());
	}

	return Storage::Reader::SetGroupResolvingKey(groupResolvingKey);
}

#endif // CONFIG_DOOR_LOCK_BLE_UWB

#ifdef CONFIG_DOOR_LOCK_CREDENTIAL_ISSUER_CA

AliroError EnsureCredentialIssuerCaPublicKey()
{
	CryptoTypes::PublicKey publicKey{};
	ReturnErrorOnFailure(DecodeHex(kCredentialIssuerPublicKeyHex, publicKey));

	const bool keyAvailable =
		Crypto::IsKeyAvailable(Storage::PsaKeyIds::kCredentialIssuerCAPublicKeyId) == ALIRO_NO_ERROR;
	if (keyAvailable) {
		CryptoTypes::PublicKey currentPublicKey{};
		ReturnErrorOnFailure(Crypto::ExportKey(Storage::PsaKeyIds::kCredentialIssuerCAPublicKeyId,
						       currentPublicKey.data(), currentPublicKey.size()));

		if (currentPublicKey == publicKey) {
			return ALIRO_NO_ERROR;
		}

		CryptoTypes::KeyId keyId{ Storage::PsaKeyIds::kCredentialIssuerCAPublicKeyId };
		ReturnErrorOnFailure(Crypto::DestroyKey(keyId));
	}

	CryptoTypes::KeyId keyId{ Storage::PsaKeyIds::kCredentialIssuerCAPublicKeyId };
	return Crypto::ImportPublicKey(publicKey, true, keyId);
}

#endif // CONFIG_DOOR_LOCK_CREDENTIAL_ISSUER_CA

} // namespace

AliroError EnsureTestParameters()
{
	LOG_WRN("Provisioning Aliro certification test parameters");

	ReturnErrorOnFailure(SavePublicKey(StorageKeys::kStorageKeyNameAccessCredentialPublicKey, 0,
					   kAccessCredentialPublicKeyHex));

#if CONFIG_DOOR_LOCK_ACCESS_MANAGER_CREDENTIAL_ISSUER_MAX_STORED_KEYS > 0
	ReturnErrorOnFailure(SavePublicKey(StorageKeys::kStorageKeyNameCredentialIssuerPublicKey, 0,
					   kCredentialIssuerPublicKeyHex));
#endif // CONFIG_DOOR_LOCK_ACCESS_MANAGER_CREDENTIAL_ISSUER_MAX_STORED_KEYS > 0

	ReturnErrorOnFailure(EnsureReaderPrivateKey());
	ReturnErrorOnFailure(EnsureReaderIdentifier());

#ifdef CONFIG_DOOR_LOCK_READER_CERTIFICATE
	ReturnErrorOnFailure(EnsureReaderIssuerPublicKey());
	ReturnErrorOnFailure(EnsureReaderCertificate());
#endif // CONFIG_DOOR_LOCK_READER_CERTIFICATE

#ifdef CONFIG_DOOR_LOCK_BLE_UWB
	ReturnErrorOnFailure(EnsureGroupResolvingKey());
#endif // CONFIG_DOOR_LOCK_BLE_UWB

#ifdef CONFIG_DOOR_LOCK_CREDENTIAL_ISSUER_CA
	ReturnErrorOnFailure(EnsureCredentialIssuerCaPublicKey());
#endif // CONFIG_DOOR_LOCK_CREDENTIAL_ISSUER_CA

	return ALIRO_NO_ERROR;
}

} // namespace DoorLock::CertificationProvisioning
