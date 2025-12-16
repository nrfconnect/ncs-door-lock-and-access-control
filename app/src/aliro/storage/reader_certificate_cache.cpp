/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "reader_certificate_cache.h"

#include "aliro/memory.h"
#include "aliro/utils.h"

#include <algorithm>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(reader_certificate_cache, CONFIG_DOOR_LOCK_APP_LOG_LEVEL);

namespace Aliro {

ReaderCertificateCache &ReaderCertificateCache::Instance()
{
	static ReaderCertificateCache instance;
	return instance;
}

bool ReaderCertificateCache::IsCertificateSet() const
{
	return mCertificate.get() != nullptr;
}

AliroError ReaderCertificateCache::SetCertificate(const Certificate &certificate)
{
	VerifyOrReturnStatus(certificate.mData != nullptr, ALIRO_INVALID_ARGUMENT, LOG_ERR("Certificate data is null"));
	VerifyOrReturnStatus(certificate.mLength != 0, ALIRO_INVALID_ARGUMENT, LOG_ERR("Certificate length is zero"));

	ClearCertificate();

	mCertificate = make_unique_array_nothrow<uint8_t>(certificate.mLength);
	VerifyOrReturnStatus(mCertificate, ALIRO_NO_MEMORY, LOG_ERR("Failed to allocate memory for certificate"));

	std::copy_n(certificate.mData, certificate.mLength, mCertificate.get());
	mCertificateLength = certificate.mLength;

	return ALIRO_NO_ERROR;
}

void ReaderCertificateCache::ClearCertificate()
{
	mCertificate.reset();
	mCertificateLength = 0;
}

bool ReaderCertificateCache::IsIssuerPublicKeySet() const
{
	return mIssuerPublicKey.has_value();
}

AliroError ReaderCertificateCache::GetCertificate(Certificate &certificate) const
{
	VerifyOrReturnStatus(mCertificate.get() != nullptr, ALIRO_ERROR_INTERNAL,
			     LOG_ERR("Reader certificate is not set"));

	certificate = { mCertificate.get(), mCertificateLength };
	return ALIRO_NO_ERROR;
}

AliroError ReaderCertificateCache::SetIssuerPublicKey(const CryptoTypes::PublicKey &publicKey)
{
	VerifyOrReturnStatus(publicKey[0] == CryptoTypes::kEccP256PublicKeyPrefix, ALIRO_INVALID_ARGUMENT,
			     LOG_ERR("Invalid issuer public key prefix"));

	mIssuerPublicKey = publicKey;
	return ALIRO_NO_ERROR;
}

AliroError ReaderCertificateCache::GetIssuerPublicKey(CryptoTypes::PublicKey &publicKey) const
{
	VerifyOrReturnStatus(mIssuerPublicKey.has_value(), ALIRO_ERROR_INTERNAL,
			     LOG_ERR("Issuer public key is not set"));

	publicKey = mIssuerPublicKey.value();
	return ALIRO_NO_ERROR;
}

void ReaderCertificateCache::ClearIssuerPublicKey()
{
	mIssuerPublicKey.reset();
}

} // namespace Aliro
