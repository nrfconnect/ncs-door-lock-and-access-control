/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

#pragma once

#include "aliro/connection_handle.h"
#include "aliro/errors.h"
#include "aliro/protocol_version.h"
#include "aliro/time.h"
#include "aliro/types.h"

#include <optional>

namespace Aliro::Interface {

namespace Access {

/**
 * @brief Parameters for the Access Document request.
 */
struct AccessDocumentRequestParams {
	/**
	 * @brief The data element identifier of the Access Document to be requested.
	 */
	ConstData mElementIdentifier;
	/**
	 * @brief Indicates the intent to store the Access Document.
	 */
	bool mIntentToStore;
};

/**
 * @brief Gets the parameters for the Access Document request.
 *
 * @param publicKey The public key of the User Device.
 * @param credentialSignedTimestamp The credential signed timestamp.
 *
 * @return The parameters for the Access Document request if the document should be requested, std::nullopt otherwise.
 */
std::optional<AccessDocumentRequestParams>
GetAccessDocumentRequestParameters(const CryptoTypes::PublicKey &publicKey,
				   const std::optional<Timestamp> &credentialSignedTimestamp);

/**
 * @brief Processes the Access Request from Expedited-standard and Step-up phases.
 *
 * @param handle The connection handle identifying the session.
 * @param userPublicKey The User Device public key.
 * @param kpersistentKeyId The volatile Kpersistent key derived during Expedited-standard phase.
 * @param accessDocument The optional Access Document presented by the User Device during Step-up phase.
 *
 * @return ALIRO_NO_ERROR when the access is granted, error code otherwise.
 */
AliroError
ProcessAccessRequest(ConnectionHandle handle, const CryptoTypes::PublicKey &userPublicKey,
		     CryptoTypes::KeyId kpersistentKeyId,
		     const std::optional<AccessDocumentTypes::AccessDocument> &accessDocument = std::nullopt);

/**
 * @brief Processes the Access Request from Expedited-fast phase.
 *
 * @param handle The connection handle identifying the session.
 * @param kpersistentKeyId The persistent Kpersistent key ID used to authenticate the User Device during Expedited-fast
 * phase.
 *
 * @return ALIRO_NO_ERROR when the access is granted, error code otherwise.
 */
AliroError ProcessAccessRequest(ConnectionHandle handle, CryptoTypes::KeyId kpersistentKeyId);

/**
 * @brief Get a Credential Issuer public key by its identifier.
 *
 * @param keyIdentifier The key identifier of the Credential Issuer public key.
 * @param publicKey The public key to get.
 *
 * @return ALIRO_NO_ERROR on success, error code on failure.
 */
AliroError GetCredentialIssuerPublicKey(const CryptoTypes::KeyIdentifier &keyIdentifier,
					CryptoTypes::PublicKey &publicKey);

/**
 * @brief Get the number of Kpersistent keys.
 *
 * @param count The number of Kpersistent keys.
 *
 * @return ALIRO_NO_ERROR when success, an error code otherwise.
 */
AliroError GetKpersistentCount(size_t &count);

/**
 * @brief Get the IDs of the Kpersistent keys.
 *
 * The caller must first query @ref GetKpersistentCount and allocate a buffer
 * large enough to hold all key IDs.
 *
 * @param keyIds Output array for Kpersistent key IDs. Must point to a writable
 * buffer with capacity for at least @p count entries when @p count is greater
 * than zero.
 * @param count Input/output number of entries in @p keyIds:
 *              - on input: capacity of @p keyIds (in number of KeyId entries),
 *              - on output: number of key IDs written to @p keyIds.
 *
 * @return ALIRO_NO_ERROR when success, an error code otherwise.
 */
AliroError GetKpersistentKeyIds(CryptoTypes::KeyId *keyIds, size_t &count);

/**
 * @brief Get the Access Credential public key corresponding to the Kpersistent key.
 *
 * @param kpersistentKeyId The ID of the Kpersistent key.
 * @param publicKey The Access Credential public key.
 *
 * @return ALIRO_NO_ERROR when success, an error code otherwise.
 */
AliroError GetAccessCredentialPublicKey(CryptoTypes::KeyId kpersistentKeyId, CryptoTypes::PublicKey &publicKey);

} // namespace Access

namespace ReaderCertificate {

/**
 * @brief Checks if the Reader certificate and Reader System Issuer public key are provisioned.
 *
 * @note Both the Reader certificate and Reader System Issuer public key must be provisioned to use the Reader
 * certificate during the Expedited-standard phase.
 *
 * @return True if the Reader certificate and Reader System Issuer public key are provisioned, false otherwise.
 */
bool IsProvisioned();

/**
 * @brief Gets the Reader System Issuer public key.
 *
 * @param publicKey The Reader System Issuer public key.
 *
 * @return ALIRO_NO_ERROR on success, error code otherwise.
 */
AliroError GetIssuerPublicKey(CryptoTypes::PublicKey &publicKey);

/**
 * @brief Gets the Reader certificate.
 *
 * @param certificate The Reader certificate.
 *
 * @return ALIRO_NO_ERROR on success, error code otherwise.
 */
AliroError GetCertificate(ConstData &certificate);

} // namespace ReaderCertificate

#ifdef CONFIG_NCS_ALIRO_BLE_UWB

namespace Ble {

/**
 * @brief BLE interface functions for Aliro stack integration.
 *
 * This section declares the functions that the application must implement
 * to integrate BLE transport with the Aliro stack. The application is responsible for:
 * - BLE stack initialization
 * - Advertising lifecycle (start/stop/update)
 * - Connection management
 *
 * These functions must be implemented in the application scope.
 */

/**
 * @brief Get the maximum number of concurrent BLE sessions supported.
 *
 * @return The maximum number of concurrent BLE sessions.
 */
size_t GetMaxSessions();

/**
 * @brief Get the BLE/UWB protocol version for the given connection.
 *
 * @param handle The connection handle to get the protocol version for.
 *
 * @return The protocol version.
 */
ProtocolVersion GetProtocolVersion(ConnectionHandle handle);

} // namespace Ble

namespace Uwb {

/**
 * @brief Handles BLE messages for the UWB module.
 *
 * This function is called by the stack to forward certain BLE messages that require
 * application-level handling. The stack performs decryption and then passes the
 * message with decrypted payload to this function.
 *
 * @note The following message types are forwarded to this function:
 *       - Notification with Ranging Message ID
 *       - UWB Ranging Service
 *
 * @param connectionHandle The connection handle identifying the BLE connection.
 * @param data Pointer to the decrypted message.
 * @param length Length of the message.
 *
 * @return ALIRO_NO_ERROR on success, error code otherwise.
 */
AliroError HandleBleMessage(ConnectionHandle connectionHandle, const uint8_t *data, size_t length);

} // namespace Uwb

#endif // CONFIG_NCS_ALIRO_BLE_UWB

namespace Session {

/**
 * @brief Send data for a session over the active transport.
 *
 * @param handle The connection handle identifying the session.
 * @param data The data to send.
 *
 * @return ALIRO_NO_ERROR on success, error code otherwise.
 */
AliroError Send(ConnectionHandle handle, Data data);

/**
 * @brief Handle a session termination event.
 *
 * Called by the stack after a session is destroyed to allow the application
 * to perform transport teardown and any application-level cleanup.
 *
 * @param handle The connection handle of the terminated session.
 */
void HandleTermination(ConnectionHandle handle);

#ifdef CONFIG_NCS_ALIRO_BLE_UWB

/**
 * @brief Starts a UWB ranging session.
 *
 * @param handle The connection handle identifying the session.
 * @param rangingSessionId The ranging session ID.
 * @param ursk The ranging session key.
 * @param protocolVersion The protocol version to use for the ranging session.
 *
 * @return ALIRO_NO_ERROR on success, error code otherwise.
 */
AliroError StartRangingSession(ConnectionHandle handle, uint32_t rangingSessionId, const CryptoTypes::Ursk &ursk,
			       ProtocolVersion protocolVersion);

#endif // CONFIG_NCS_ALIRO_BLE_UWB

} // namespace Session

namespace CredentialIssuerCertificate {

/**
 * @brief Structure representing certificate validity timestamps.
 */
struct CertificateTimestamps {
	Time mValidFrom;
	Time mValidUntil;
};

/**
 * @brief Validates an X.509 certificate and extracts the public key and timestamps.
 *
 * @param certificate The DER-encoded certificate to validate.
 * @param publicKey Output parameter for the public key extracted from the certificate.
 * @param timestamps Output parameter for the certificate validity timestamps (valid from/until).
 *
 * @return ALIRO_NO_ERROR on success, error code otherwise.
 */
AliroError Validate(const ConstData &certificate, CryptoTypes::PublicKey &publicKey,
		    std::optional<CertificateTimestamps> &timestamps);

} // namespace CredentialIssuerCertificate

namespace AccessDocument {

/**
 * @brief Verifies the current time against a validity period.
 *
 * Checks whether the current time falls within the specified validity period
 * defined by the validFrom and validUntil timestamps.
 *
 * @param validFrom The start of the validity period (inclusive).
 * @param validUntil The end of the validity period (inclusive).
 *
 * @return True if the current time is within the validity period, false otherwise.
 *         std::nullopt if the verification is not possible.
 */
std::optional<bool> VerifyValidityPeriod(const Time &validFrom, const Time &validUntil);

} // namespace AccessDocument

namespace Crypto {

/**
 * @brief Generate random bytes.
 *
 * @param buffer Output buffer for storing the generated bytes.
 * @param bufferLength Input number of bytes to generate.
 *
 * @return ALIRO_NO_ERROR on success, error status otherwise.
 */
AliroError GenerateRandom(uint8_t *buffer, size_t bufferLength);

/**
 * @brief Generate ephemeral EC key pair.
 *
 * @param keyId Output identifier for newly created keys.
 * @param ephemeralPubKey Output buffer where the public key is to be copied.
 *
 * @return ALIRO_NO_ERROR on success, error status otherwise.
 */
AliroError GenerateEphemeralKeyPair(CryptoTypes::KeyId &keyId, CryptoTypes::PublicKey &ephemeralPubKey);

/**
 * @brief Import a shared key.
 *
 * @param key input buffer with the key.
 * @param keyLength length of the key in bytes.
 * @param keyId output identifier of the imported key.
 *
 * @return ALIRO_NO_ERROR on success, error status otherwise.
 */
AliroError ImportSharedKey(const uint8_t *key, size_t keyLength, CryptoTypes::KeyId &keyId);

/**
 * @brief Import a symmetric key.
 *
 * @param key input buffer with the key.
 * @param keyLength length of the key in bytes.
 * @param keyId output identifier of the imported key.
 *
 * @return ALIRO_NO_ERROR on success, error status otherwise.
 */
AliroError ImportSymmetricKey(const uint8_t *key, size_t keyLength, CryptoTypes::KeyId &keyId);

/**
 * @brief Destroy an key by ID.
 *
 * @param keyId identifier of a key to delete.
 * NOTE: The function may set a ID to value 0.
 *
 * @return ALIRO_NO_ERROR on success, error status otherwise.
 */
AliroError DestroyKey(CryptoTypes::KeyId &keyId);

/**
 * @brief Generate signature for a message.
 *
 * @param keyId input identifier of the private key to use for signing.
 * @param msg input message to sign.
 * @param msgLength input size of the message.
 * @param signature output buffer where the signature is to be copied.
 *
 * @return ALIRO_NO_ERROR on success, error status otherwise.
 */
AliroError GenerateSignature(CryptoTypes::KeyId keyId, const uint8_t *msg, const size_t msgLength,
			     CryptoTypes::Signature &signature);

/**
 * @brief Verify signature of a message.
 *
 * @param publicKey input public key to use for verification.
 * @param msg input message whose signature is to be verified.
 * @param msgLength input size of the message.
 * @param signature input signature to verify.
 *
 * @return ALIRO_NO_ERROR when signature is valid, ALIRO_INVALID_SIGNATURE otherwise.
 */
AliroError VerifySignature(const CryptoTypes::PublicKey &publicKey, const uint8_t *msg, const size_t msgLength,
			   const CryptoTypes::Signature &signature);

/**
 * @brief Perform a raw key agreement.
 *
 * @param keyId input identifier of the private key to use for key agreement.
 * @param peerPublicKey input public key of the peer to use for key agreement.
 * @param sharedSecret output buffer where the shared secret is to be copied.
 *
 * @return ALIRO_NO_ERROR on success, error status otherwise.
 */
AliroError RawKeyAgreement(CryptoTypes::KeyId keyId, const CryptoTypes::PublicKey &peerPublicKey,
			   CryptoTypes::SharedSecret &sharedSecret);

/**
 * @brief Derive a shared key.
 *
 * @param keyId input identifier of the key to use for derivation.
 * @param info input additional info for the derivation.
 * @param infoLength input size of the info.
 * @param salt input salt for the derivation.
 * @param saltLength input size of the salt.
 * @param outputKeyId output identifier of the derived key.
 *
 * @return ALIRO_NO_ERROR on success, error status otherwise.
 */
AliroError DeriveSharedKey(CryptoTypes::KeyId keyId, const uint8_t *info, size_t infoLength, const uint8_t *salt,
			   size_t saltLength, CryptoTypes::KeyId &outputKeyId);

/**
 * @brief Derive a symmetric key.
 *
 * @param keyId input identifier of the key to use for derivation.
 * @param info input additional info for the derivation.
 * @param infoLength input size of the info.
 * @param salt input salt for the derivation.
 * @param saltLength input size of the salt.
 * @param outputKeyId output identifier of the derived key.
 *
 * @return ALIRO_NO_ERROR on success, error status otherwise.
 */
AliroError DeriveSymmetricKey(CryptoTypes::KeyId keyId, const uint8_t *info, size_t infoLength, const uint8_t *salt,
			      size_t saltLength, CryptoTypes::KeyId &outputKeyId);

/**
 * @brief Derive a key raw.
 *
 * @param keyId input identifier of the key to use for derivation.
 * @param info input additional info for the derivation.
 * @param infoLength input size of the info.
 * @param salt input salt for the derivation.
 * @param saltLength input size of the salt.
 * @param outputKey output buffer for the derived key.
 * @param outputKeyLength input size of the output key.
 *
 * @return ALIRO_NO_ERROR on success, error status otherwise.
 */
AliroError DeriveRawKey(CryptoTypes::KeyId keyId, const uint8_t *info, size_t infoLength, const uint8_t *salt,
			size_t saltLength, uint8_t *outputKey, size_t outputKeyLength);

/**
 * @brief Authenticated encryption with additional data.
 *
 * @param keyId input identifier of the key to use for encryption.
 * @param plainTxt input raw payload to encrypt.
 * @param plainTxtLength input size of the payload.
 * @param additionalData input additional data.
 * @param additionalDataLength input size of the additional data.
 * @param nonce input a nonce to use for operation.
 * @param cipherText output encrypted payload.
 * @param authTag output authentication tag.
 *
 * @return ALIRO_NO_ERROR on success, error status otherwise.
 */
AliroError AeadEncrypt(CryptoTypes::KeyId keyId, const uint8_t *plainTxt, size_t plainTxtLength,
		       const uint8_t *additionalData, size_t additionalDataLength, const CryptoTypes::Nonce &nonce,
		       uint8_t *cipherText, CryptoTypes::AuthenticationTag &authTag);

/**
 * @brief Authenticated decryption with additional data.
 *
 * @param keyId input identifier of the key to use for decryption.
 * @param cipherTextWithTag input encrypted and authenticated data. The buffer must contains the encrypted data
 * followed by authentication tag.
 * @param cipherTextWithTagLength input size of the cipherText buffer.
 * @param additionalData input additional data that has been authenticated but not encrypted.
 * @param additionalDataLength input size of the additional data.
 * @param nonce input a nonce to use for operation with fixed size.
 * @param plainText output buffer for decrypted data.
 * @param plainTextLength input/output size of the plainText buffer. On success the size of the output after
 * decryption.
 *
 * @return ALIRO_NO_ERROR on success, error status otherwise.
 */
AliroError AeadDecrypt(CryptoTypes::KeyId keyId, const uint8_t *cipherTextWithTag, size_t cipherTextWithTagLength,
		       const uint8_t *additionalData, size_t additionalDataLength, const CryptoTypes::Nonce &nonce,
		       uint8_t *plainText, size_t &plainTextLength);

/**
 * @brief Encrypt data.
 *
 * @param keyId input identifier of the key to use for encryption.
 * @param plainTxt input raw payload to encrypt.
 * @param plainTxtLength input size of the payload.
 * @param cipherText output encrypted payload.
 *
 * @return ALIRO_NO_ERROR on success, error status otherwise.
 */
AliroError Encrypt(CryptoTypes::KeyId keyId, const uint8_t *plainTxt, size_t plainTxtLength, uint8_t *cipherText);

/**
 * @brief Computes the SHA-256 hash of the data.
 *
 * @param data The data to compute the hash of.
 * @param dataLength The length of the data.
 * @param hash The computed SHA-256 hash of the data.
 *
 * @return ALIRO_NO_ERROR on success, error status otherwise.
 */
AliroError Sha256(const uint8_t *data, size_t dataLength, CryptoTypes::Sha256Hash &hash);

} // namespace Crypto

} // namespace Aliro::Interface
