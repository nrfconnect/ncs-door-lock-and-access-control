/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include "aliro/errors.h"
#include "aliro/types.h"

#include <memory>
#include <optional>

namespace Aliro {

/**
 * @brief Singleton class for caching the Reader certificate.
 *
 * This class manages the Reader certificate that is sent during the LOAD_CERT command.
 * The certificate can be provisioned via CLI and is stored in RAM for quick access.
 * If no certificate is provisioned, the LOAD_CERT state is skipped.
 */
class ReaderCertificateCache {
public:
	/**
	 * @brief Type alias for certificate data.
	 */
	using Certificate = ConstData;

	/**
	 * @brief Gets the singleton instance.
	 *
	 * @return Reference to the singleton instance.
	 */
	static ReaderCertificateCache &Instance();

	/**
	 * @brief Checks if the Reader certificate is set.
	 *
	 * @return True if the Reader certificate is set, false otherwise.
	 */
	bool IsCertificateSet() const;

	/**
	 * @brief Sets the Reader certificate.
	 *
	 * @param certificate ConstData containing the certificate data.
	 * @return ALIRO_NO_ERROR on success, error code otherwise.
	 */
	AliroError SetCertificate(const Certificate &certificate);

	/**
	 * @brief Clears the Reader certificate.
	 */
	void ClearCertificate();

	/**
	 * @brief Checks if the Issuer public key is set.
	 *
	 * @return True if the Issuer public key is set, false otherwise.
	 */
	bool IsIssuerPublicKeySet() const;

	/**
	 * @brief Gets the Reader certificate.
	 *
	 * @param certificate The Reader certificate.
	 *
	 * @return ALIRO_NO_ERROR on success, error code otherwise.
	 */
	AliroError GetCertificate(Certificate &certificate) const;

	/**
	 * @brief Sets the Issuer public key.
	 *
	 * @param publicKey The Issuer public key.
	 *
	 * @return ALIRO_NO_ERROR on success, error code otherwise.
	 */
	AliroError SetIssuerPublicKey(const CryptoTypes::PublicKey &publicKey);

	/**
	 * @brief Gets the Issuer public key.
	 *
	 * @param publicKey The Issuer public key.
	 *
	 * @return ALIRO_NO_ERROR on success, error code otherwise.
	 */
	AliroError GetIssuerPublicKey(CryptoTypes::PublicKey &publicKey) const;

	/**
	 * @brief Clears the Issuer public key.
	 */
	void ClearIssuerPublicKey();

private:
	ReaderCertificateCache() = default;
	~ReaderCertificateCache() = default;

	ReaderCertificateCache(const ReaderCertificateCache &) = delete;
	ReaderCertificateCache &operator=(const ReaderCertificateCache &) = delete;
	ReaderCertificateCache(ReaderCertificateCache &&) = delete;
	ReaderCertificateCache &operator=(ReaderCertificateCache &&) = delete;

	std::unique_ptr<uint8_t[]> mCertificate{};
	size_t mCertificateLength{ 0 };
	std::optional<CryptoTypes::PublicKey> mIssuerPublicKey{};
};

} // namespace Aliro
