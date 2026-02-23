/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "reader_cache.h"

#include "aliro/memory.h"
#include "aliro/utils.h"

#include <algorithm>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(reader_cache, CONFIG_DOOR_LOCK_APP_LOG_LEVEL);

namespace Aliro {

ReaderCache &ReaderCache::Instance()
{
	static ReaderCache instance;
	return instance;
}

bool ReaderCache::IsIdentifierSet() const
{
	return mIdentifier.has_value();
}

AliroError ReaderCache::SetIdentifier(const Identifier &identifier)
{
	mIdentifier = identifier;
	return ALIRO_NO_ERROR;
}

AliroError ReaderCache::GetIdentifier(Identifier &identifier) const
{
	VerifyOrReturnStatus(mIdentifier.has_value(), ALIRO_ERROR_INTERNAL, LOG_ERR("Reader identifier is not set"));

	identifier = mIdentifier.value();
	return ALIRO_NO_ERROR;
}

void ReaderCache::ClearIdentifier()
{
	mIdentifier.reset();
}

AliroError ReaderCache::SetPublicKey(const CryptoTypes::PublicKey &publicKey)
{
	mPublicKey = publicKey;
	return ALIRO_NO_ERROR;
}

AliroError ReaderCache::GetPublicKey(CryptoTypes::PublicKey &publicKey) const
{
	VerifyOrReturnStatus(mPublicKey.has_value(), ALIRO_ERROR_INTERNAL, LOG_ERR("Reader public key is not set"));

	publicKey = mPublicKey.value();
	return ALIRO_NO_ERROR;
}

void ReaderCache::ClearPublicKey()
{
	mPublicKey.reset();
}

bool ReaderCache::IsCertificateSet() const
{
	return mCertificate.get() != nullptr;
}

AliroError ReaderCache::SetCertificate(const Certificate &certificate)
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

void ReaderCache::ClearCertificate()
{
	mCertificate.reset();
	mCertificateLength = 0;
}

bool ReaderCache::IsIssuerPublicKeySet() const
{
	return mIssuerPublicKey.has_value();
}

AliroError ReaderCache::GetCertificate(Certificate &certificate) const
{
	VerifyOrReturnStatus(mCertificate.get() != nullptr, ALIRO_ERROR_INTERNAL,
			     LOG_ERR("Reader certificate is not set"));

	certificate = { mCertificate.get(), mCertificateLength };
	return ALIRO_NO_ERROR;
}

AliroError ReaderCache::SetIssuerPublicKey(const CryptoTypes::PublicKey &publicKey)
{
	VerifyOrReturnStatus(publicKey[0] == CryptoTypes::kEccP256PublicKeyPrefix, ALIRO_INVALID_ARGUMENT,
			     LOG_ERR("Invalid issuer public key prefix"));

	mIssuerPublicKey = publicKey;
	return ALIRO_NO_ERROR;
}

AliroError ReaderCache::GetIssuerPublicKey(CryptoTypes::PublicKey &publicKey) const
{
	VerifyOrReturnStatus(mIssuerPublicKey.has_value(), ALIRO_ERROR_INTERNAL,
			     LOG_ERR("Issuer public key is not set"));

	publicKey = mIssuerPublicKey.value();
	return ALIRO_NO_ERROR;
}

void ReaderCache::ClearIssuerPublicKey()
{
	mIssuerPublicKey.reset();
}

} // namespace Aliro
