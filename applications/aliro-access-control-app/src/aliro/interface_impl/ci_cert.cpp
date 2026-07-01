/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <aliro/interface.h>

#ifdef CONFIG_DOOR_LOCK_CREDENTIAL_ISSUER_CA

#include <aliro/utils.h>

#include "psa_key_ids.h"

#include <crypto_utils/crypto_utils.h>

extern "C" {
#include <mbedtls/psa_util.h>
}

#include <mbedtls/oid.h>
#include <mbedtls/x509_crt.h>
#include <zephyr/logging/log.h>

#include <algorithm>

LOG_MODULE_REGISTER(interface_ci_cert, CONFIG_DOOR_LOCK_APP_LOG_LEVEL);

#endif // CONFIG_DOOR_LOCK_CREDENTIAL_ISSUER_CA

namespace Aliro::Interface::CredentialIssuerCertificate {

#ifdef CONFIG_DOOR_LOCK_CREDENTIAL_ISSUER_CA

namespace {

AliroError VerifyKeyType(const mbedtls_pk_context *pk)
{
	constexpr size_t kEccP256KeyBits{ CryptoTypes::kEccP256KeyPrivateKeyLength * BITS_PER_BYTE };
	psa_key_attributes_t attributes{};
	mbedtls_pk_get_psa_attributes(pk, PSA_KEY_USAGE_VERIFY_HASH, &attributes);

	psa_key_type_t type = psa_get_key_type(&attributes);
	VerifyOrReturnStatus(type == PSA_KEY_TYPE_ECC_PUBLIC_KEY(PSA_ECC_FAMILY_SECP_R1), ALIRO_INVALID_DATA_CONTENT,
			     LOG_ERR("Invalid public key type"));
	VerifyOrReturnStatus(psa_get_key_bits(&attributes) == kEccP256KeyBits, ALIRO_INVALID_DATA_CONTENT,
			     LOG_ERR("Invalid public key bit length"));

	return ALIRO_NO_ERROR;
}

AliroError VerifyCertificateSignature(const mbedtls_x509_crt &crt)
{
	constexpr size_t kCoordinateBits{ CryptoTypes::kEccP256KeySingleCoordinateLength * BITS_PER_BYTE };
	CryptoTypes::Signature signatureArray{};
	size_t rawLength{};

	auto result =
		mbedtls_ecdsa_der_to_raw(kCoordinateBits, crt.MBEDTLS_PRIVATE(sig).p, crt.MBEDTLS_PRIVATE(sig).len,
					 signatureArray.data(), signatureArray.size(), &rawLength);
	VerifyOrReturnStatus(result == 0, ALIRO_ERROR_INTERNAL, LOG_ERR("Failed to convert signature from DER to raw"));
	VerifyOrReturnStatus(rawLength == signatureArray.size(), ALIRO_INVALID_DATA_FORMAT,
			     LOG_ERR("Invalid signature length"));

	return DoorLock::CryptoUtils::VerifySignature(DoorLock::Storage::PsaKeyIds::kCredentialIssuerCAPublicKeyId,
						      crt.tbs.p, crt.tbs.len, signatureArray);
}

AliroError VerifyCertificate(const mbedtls_x509_crt &crt)
{
	VerifyOrReturnStatus(crt.version == 3, ALIRO_INVALID_DATA_FORMAT, LOG_ERR("Invalid certificate version"));

	auto result = mbedtls_x509_crt_has_ext_type(&crt, MBEDTLS_X509_EXT_KEY_USAGE);
	VerifyOrReturnStatus(result != 0, ALIRO_INVALID_DATA_CONTENT, LOG_ERR("Key usage extension is not present"));
	result = mbedtls_x509_crt_check_key_usage(&crt, MBEDTLS_X509_KU_DIGITAL_SIGNATURE);
	VerifyOrReturnStatus(result == 0, ALIRO_INVALID_DATA_CONTENT, LOG_ERR("Digital Signature bit not set"));

	auto error = VerifyKeyType(&crt.pk);
	VerifyOrReturnStatus(error == ALIRO_NO_ERROR, error, LOG_ERR("Failed to verify key type"));

	VerifyOrReturnStatus(crt.MBEDTLS_PRIVATE(sig_md) == MBEDTLS_MD_SHA256, ALIRO_INVALID_DATA_CONTENT,
			     LOG_ERR("Invalid signature algorithm"));
	VerifyOrReturnStatus(crt.MBEDTLS_PRIVATE(sig_pk) == MBEDTLS_PK_SIGALG_ECDSA, ALIRO_INVALID_DATA_CONTENT,
			     LOG_ERR("Invalid signature algorithm"));

	error = VerifyCertificateSignature(crt);
	VerifyOrReturnStatus(error == ALIRO_NO_ERROR, error, LOG_ERR("Failed to verify certificate signature"));

	return ALIRO_NO_ERROR;
}

AliroError VerifyCertificateValidityPeriod(CertificateTimestamps &timestamps)
{
	const auto &validFrom = timestamps.mValidFrom;
	const auto &validUntil = timestamps.mValidUntil;

	LOG_DBG("validFrom   : %04d-%02d-%02d %02d:%02d:%02d", validFrom.mYear, validFrom.mMonth, validFrom.mDay,
		validFrom.mHour, validFrom.mMinute, validFrom.mSecond);
	LOG_DBG("validUntil  : %04d-%02d-%02d %02d:%02d:%02d", validUntil.mYear, validUntil.mMonth, validUntil.mDay,
		validUntil.mHour, validUntil.mMinute, validUntil.mSecond);

	VerifyOrReturnStatus(validFrom <= validUntil, ALIRO_INVALID_ARGUMENT, LOG_ERR("Invalid validity period"));

	LOG_WRN("Time concept is not supported, skipping validity period verification");

	return ALIRO_NO_ERROR;
}

} // namespace

AliroError Validate(const ConstData &certificate, CryptoTypes::PublicKey &publicKey,
		    std::optional<CertificateTimestamps> &timestamps)
{
#ifndef MBEDTLS_PK_USE_PSA_EC_DATA

	mbedtls_ecp_keypair *keypair{ nullptr };
	size_t pubkey_size{ 0 };

#endif // MBEDTLS_PK_USE_PSA_EC_DATA

	VerifyOrReturnStatus(certificate.mData != nullptr, ALIRO_INVALID_ARGUMENT, LOG_ERR("Certificate data is null"));
	VerifyOrReturnStatus(certificate.mLength != 0, ALIRO_INVALID_ARGUMENT, LOG_ERR("Certificate length is zero"));

	mbedtls_x509_crt crt;
	mbedtls_x509_crt_init(&crt);

	auto result = mbedtls_x509_crt_parse_der_nocopy(&crt, certificate.mData, certificate.mLength);
	AliroError error = (result == 0) ? ALIRO_NO_ERROR : ALIRO_INVALID_DATA_FORMAT;
	VerifyOrExit(result == 0, LOG_ERR("Failed to parse certificate"));

	error = VerifyCertificate(crt);
	VerifyOrExit(error == ALIRO_NO_ERROR, LOG_ERR("Failed to verify certificate"));

#ifndef MBEDTLS_PK_USE_PSA_EC_DATA

	keypair = mbedtls_pk_ec(crt.pk);
	result = mbedtls_ecp_point_write_binary(&keypair->MBEDTLS_PRIVATE(grp), &keypair->MBEDTLS_PRIVATE(Q),
						MBEDTLS_ECP_PF_UNCOMPRESSED, &pubkey_size, publicKey.data(),
						publicKey.size());
	VerifyOrExit(result == 0, error = ALIRO_INVALID_ARGUMENT; LOG_ERR("Failed to extract public key"));
	VerifyOrExit(pubkey_size == CryptoTypes::kEccP256PublicKeyLength, error = ALIRO_INVALID_ARGUMENT;
		     LOG_ERR("Invalid public key length"));

#else // MBEDTLS_PK_USE_PSA_EC_DATA

	VerifyOrExit(crt.pk.MBEDTLS_PRIVATE(pub_raw_len) == CryptoTypes::kEccP256PublicKeyLength,
		     error = ALIRO_INVALID_DATA_FORMAT;
		     LOG_ERR("Invalid public key length"));

	std::copy_n(crt.pk.MBEDTLS_PRIVATE(pub_raw), crt.pk.MBEDTLS_PRIVATE(pub_raw_len), publicKey.data());

#endif // MBEDTLS_PK_USE_PSA_EC_DATA

	timestamps.emplace(
		CertificateTimestamps{ .mValidFrom = Time(crt.valid_from.year, crt.valid_from.mon, crt.valid_from.day,
							  crt.valid_from.hour, crt.valid_from.min, crt.valid_from.sec),
				       .mValidUntil = Time(crt.valid_to.year, crt.valid_to.mon, crt.valid_to.day,
							   crt.valid_to.hour, crt.valid_to.min, crt.valid_to.sec) });

	error = VerifyCertificateValidityPeriod(timestamps.value());
	VerifyOrExit(error == ALIRO_NO_ERROR, LOG_ERR("Failed to verify certificate validity period"));

exit:
	mbedtls_x509_crt_free(&crt);
	return error;
}

#else

AliroError Validate(const ConstData &, CryptoTypes::PublicKey &, std::optional<CertificateTimestamps> &)
{
	return ALIRO_ERROR_NOT_IMPLEMENTED;
}

#endif // CONFIG_DOOR_LOCK_CREDENTIAL_ISSUER_CA

} // namespace Aliro::Interface::CredentialIssuerCertificate
