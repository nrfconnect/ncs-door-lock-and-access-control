/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "aliro/interface.h"

#include "reader_certificate_cache.h"

namespace Aliro::Interface::ReaderCertificate {

bool IsProvisioned()
{
	return ReaderCertificateCache::Instance().IsCertificateSet() &&
	       ReaderCertificateCache::Instance().IsIssuerPublicKeySet();
}

AliroError GetIssuerPublicKey(CryptoTypes::PublicKey &publicKey)
{
	return ReaderCertificateCache::Instance().GetIssuerPublicKey(publicKey);
}

AliroError GetCertificate(ConstData &certificate)
{
	return ReaderCertificateCache::Instance().GetCertificate(certificate);
}

} // namespace Aliro::Interface::ReaderCertificate
