/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include <aliro/errors.h>
#include <aliro/types.h>

#include <cstddef>

namespace Aliro {

AliroError StoreCertificateCredentialIssuerKey(size_t index, const CryptoTypes::PublicKey &publicKey,
					       const ValidityPeriod &validityPeriod);
AliroError ReadCertificateCredentialIssuerKey(size_t index, CryptoTypes::PublicKey &publicKey,
					      ValidityPeriod *validityPeriod = nullptr);
AliroError ClearCertificateCredentialIssuerKey(size_t index);
AliroError LoadCertificateCredentialIssuerKeys();

} // namespace Aliro
