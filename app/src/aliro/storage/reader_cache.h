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
 * @brief Singleton class for caching Reader data.
 *
 * This class manages Reader provisioning data used by the application:
 * - Reader identifier
 * - Reader public key
 * - Reader certificate
 * - Reader System Issuer public key
 */
class ReaderCache {
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
	static ReaderCache &Instance();

	/**
	 * @brief Checks if the Reader identifier is set.
	 *
	 * @return True if the Reader identifier is set, false otherwise.
	 */
	bool IsIdentifierSet() const;

	/**
	 * @brief Sets the Reader identifier.
	 *
	 * @param identifier The Reader identifier.
	 *
	 * @return ALIRO_NO_ERROR on success, error code otherwise.
	 */
	AliroError SetIdentifier(const Identifier &identifier);

	/**
	 * @brief Gets the Reader identifier.
	 *
	 * @param identifier The Reader identifier.
	 *
	 * @return ALIRO_NO_ERROR on success, error code otherwise.
	 */
	AliroError GetIdentifier(Identifier &identifier) const;

	/**
	 * @brief Clears the Reader identifier.
	 */
	void ClearIdentifier();

	/**
	 * @brief Sets the Reader public key.
	 *
	 * @param publicKey The Reader public key.
	 *
	 * @return ALIRO_NO_ERROR on success, error code otherwise.
	 */
	AliroError SetPublicKey(const CryptoTypes::PublicKey &publicKey);

	/**
	 * @brief Gets the Reader public key.
	 *
	 * @param publicKey The Reader public key.
	 *
	 * @return ALIRO_NO_ERROR on success, error code otherwise.
	 */
	AliroError GetPublicKey(CryptoTypes::PublicKey &publicKey) const;

	/**
	 * @brief Clears the Reader public key.
	 */
	void ClearPublicKey();

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
	ReaderCache() = default;
	~ReaderCache() = default;

	ReaderCache(const ReaderCache &) = delete;
	ReaderCache &operator=(const ReaderCache &) = delete;
	ReaderCache(ReaderCache &&) = delete;
	ReaderCache &operator=(ReaderCache &&) = delete;

	std::optional<Identifier> mIdentifier{};
	std::unique_ptr<uint8_t[]> mCertificate{};
	size_t mCertificateLength{ 0 };
	std::optional<CryptoTypes::PublicKey> mIssuerPublicKey{};
	std::optional<CryptoTypes::PublicKey> mPublicKey{};
};

} // namespace Aliro
