/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include "aliro/errors.h"
#include "aliro/types.h"

namespace Aliro {

/**
 * @brief Structure representing an Access Document stored in persistent storage.
 *
 * This structure contains all the metadata and data associated with an Access Document.
 */
struct AccessDocument {
	/** @brief Maximum size of the Access Document data buffer. */
	static constexpr size_t kAccessDocumentSize{ CONFIG_DOOR_LOCK_STORAGE_MAX_STORED_ACCESS_DOCUMENT_SIZE };
	/** @brief Current version of the Access Document structure. */
	static constexpr uint32_t kVersion{ 1 };

	/** @brief Version of the Access Document structure that is stored in persistent storage. */
	uint32_t mVersion;
	/** @brief Index of the Credential Issuer key associated with this Access Document. */
	size_t mCredentialIssuerKeyIndex;
	/** @brief Timestamp when the Access Document was signed. */
	Aliro::Timestamp mSignedTimestamp;
	/** @brief Validity iteration number for this Access Document. */
	ValidityIteration mAccessIteration;
	/** @brief Access Credential public key associated with this Access Document. */
	Aliro::CryptoTypes::PublicKey mPublicKey;
	/** @brief Actual size of the Access Document data in the buffer. */
	size_t mAccessDocumentSize;
	/** @brief Buffer containing the actual Access Document data. */
	std::array<uint8_t, kAccessDocumentSize> mAccessDocument;
};

/**
 * @brief Loads all Access Documents from persistent storage.
 *
 * @return ALIRO_NO_ERROR on success, or an error code on failure.
 */
AliroError LoadAccessDocuments();

/**
 * @brief Reads an Access Document from persistent storage.
 *
 * @param index The index of the Access Document to read.
 * @param ad Reference to the AccessDocument structure that will contain the read Access Document data.
 *
 * @return ALIRO_NO_ERROR on success, or an error code on failure.
 */
AliroError ReadAccessDocument(size_t index, AccessDocument &ad);

/**
 * @brief Stores an Access Document to persistent storage.
 *
 * @param index The index of the Access Document to store.
 * @param ad Reference to the AccessDocument structure to store.
 *
 * @return ALIRO_NO_ERROR on success, or an error code on failure.
 */
AliroError StoreAccessDocument(size_t index, const AccessDocument &ad);

/**
 * @brief Clears an Access Document from persistent storage.
 *
 * @param index The index of the Access Document to clear.
 * @param updateUser Indicates if the related credential list in Matter User cache should be updated.
 *
 * @return ALIRO_NO_ERROR on success, or an error code on failure.
 */
AliroError ClearAccessDocument(size_t index, bool updateUser);

} // namespace Aliro
