/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

#pragma once

#include "aliro/connection_handle.h"
#include "aliro/errors.h"
#include "aliro/protocol_version.h"
#include "aliro/types.h"

namespace Aliro {

class AccessManagerImpl;

/**
 * @brief AccessManager An interface for making access control decisions.
 *
 * This class is an API that should manage User Device public keys and makes access decisions based on:
 * - Signature verification status
 * - Public key validation against stored keys
 * - UWB ranging data analysis
 */
class AccessManager {
public:
	using SessionContext = ConnectionHandle;
	using LockIndicatorCallback = void (*)(OperationSource source);
	using AccessIndicatorCallback = void (*)(bool isAccessGranted, bool isNfcSession);
	using TerminateSessionCallback = void (*)(SessionContext sessionContext);

	enum class PublicKeyType : uint8_t {
		AccessCredential = 0,
		CredentialIssuer = 1,
		AccessDocument = 2,
	};

	/**
	 * @brief Application callbacks.
	 */
	struct ApplicationCallbacks {
		/**
		 * @brief Callback for signaling unlock action.
		 *
		 * This callback is called when access is granted and the door should be unlocked.
		 */
		LockIndicatorCallback mUnlockIndicatorClb{ nullptr };

		/**
		 * @brief Callback for signaling lock action.
		 *
		 * This callback is called when access is granted and the door should be locked.
		 */
		LockIndicatorCallback mLockIndicatorClb{ nullptr };

		/**
		 * @brief Callback for signaling access action.
		 *
		 * This callback is called when the access is granted or denied.
		 * This callback can be used for security logs.
		 */
		AccessIndicatorCallback mAccessIndicatorClb{ nullptr };
	};

	/**
	 * @brief Stack callbacks.
	 *
	 */
	struct StackCallbacks {
		/**
		 * @brief Callback for terminating the Aliro session.
		 *
		 * This callback is called when the session should be terminated.
		 */
		TerminateSessionCallback mTerminateSessionClb{ nullptr };
	};

	/**
	 * @brief Set the application callbacks for the AccessManager.
	 *
	 * @param callbacks Callbacks.
	 */
	void SetApplicationCallbacks(const ApplicationCallbacks &callbacks);

	/**
	 * @brief Set the stack callbacks.
	 *
	 * @param callbacks Stack callbacks.
	 */
	void SetStackCallbacks(const StackCallbacks &callbacks);

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
	 * @brief Checks if the Access Document for a parameters should be requested.
	 *
	 * @param publicKey The public key of the User Device.
	 * @param credentialSignedTimestamp The credential signed timestamp.
	 * @param result The parameters for the request.
	 *
	 * @return The parameter for the request if the Access Document should be requested, std::nullopt otherwise.
	 */
	std::optional<AccessDocumentRequestParams>
	ShouldRequestAccessDocument(const CryptoTypes::PublicKey &publicKey,
				    const std::optional<Timestamp> &credentialSignedTimestamp);

	/**
	 * @brief Verifies the Access Credential based on provided inputs.
	 *
	 * @param userPublicKey The User Device public key to verify.
	 * @param isNfcSession Indicates if the session is a NFC session.
	 * @param sessionContext A pointer to the session context.
	 * @param accessDocument The access document provided by the User Device.
	 *
	 * @return ALIRO_NO_ERROR on success, error code otherwise.
	 */
	AliroError
	VerifyAccessCredential(const CryptoTypes::PublicKey &userPublicKey, bool isNfcSession,
			       SessionContext sessionContext,
			       const std::optional<AccessDocumentTypes::AccessDocument> &accessDocument = std::nullopt);

	/**
	 * @brief Verifies the Kpersistent key based on provided inputs.
	 *
	 * @param kpersistentKeyId The Kpersistent key ID to verify.
	 * @param isNfcSession Indicates if the session is a NFC session.
	 * @param sessionContext A pointer to the session context.
	 *
	 * @return ALIRO_NO_ERROR on success, error code otherwise.
	 */
	AliroError VerifyKPersistentKey(CryptoTypes::KeyId kpersistentKeyId, bool isNfcSession,
					SessionContext sessionContext);

#ifdef CONFIG_NCS_ALIRO_BLE_UWB
	/**
	 * @brief Starts a ranging session based on provided inputs.
	 *
	 * @param rangingSessionId The ranging session ID.
	 * @param ursk The ranging session key.
	 * @param protocolVersion The protocol version to use for the ranging session.
	 * @param sessionContext A pointer to the session context.
	 *
	 * @return ALIRO_NO_ERROR on success, error code otherwise.
	 */
	AliroError StartRangingSession(uint32_t rangingSessionId, const CryptoTypes::Ursk &ursk,
				       ProtocolVersion protocolVersion, SessionContext sessionContext);
#endif // CONFIG_NCS_ALIRO_BLE_UWB

	/**
	 * @brief Add a new public key to the AccessManager.
	 *
	 * @param publicKey The User Device public key to add.
	 * @param publicKeyType The type of the public key.
	 * @param keyIndex The index of the public key in the storage.
	 *
	 * @return ALIRO_NO_ERROR on success, error code on failure.
	 */
	AliroError AddPublicKey(const CryptoTypes::PublicKey &publicKey, PublicKeyType publicKeyType, size_t keyIndex);

	/**
	 * @brief Check if a public key is stored in the AccessManager.
	 *
	 * @param publicKey The public key to check.
	 * @param keyIndex The index of the public key in the storage if the public key is stored.
	 *
	 * @return True if the public key is stored, false otherwise.
	 */
	bool IsPublicKeyStored(const CryptoTypes::PublicKey &publicKey, size_t *keyIndex = nullptr);

	/**
	 * @brief Get a public key from the AccessManager by its index.
	 *
	 * @param keyIndex The index of the public key in the storage.
	 * @param publicKey The public key to get.
	 *
	 * @return ALIRO_NO_ERROR on success, error code on failure.
	 */
	AliroError GetPublicKey(size_t keyIndex, CryptoTypes::PublicKey &publicKey);

	/**
	 * @brief Remove a public key from the AccessManager.
	 *
	 * @param publicKeyType The type of the public key.
	 * @param keyIndex The index of the public key in the storage.
	 *
	 * @return ALIRO_NO_ERROR on success, error code on failure.
	 */
	AliroError RemovePublicKey(PublicKeyType publicKeyType, size_t keyIndex);

	/**
	 * @brief Get a Credential Issuer public key by its identifier.
	 *
	 * @param keyIdentifier The key identifier of the Credential Issuer public key.
	 * @param publicKey The public key to get.
	 *
	 * @return ALIRO_NO_ERROR on success, error code on failure.
	 */
	AliroError GetCredentialIssuerPublicKey(const CryptoTypes::KeyIdentifier &keyIdentifier,
						CryptoTypes::PublicKey &publicKey) const;

	/**
	 * @brief Clear all stored public keys.
	 */
	void ClearStoredKeys();

	/**
	 * @brief Set the maximum allowed distance for UWB ranging (in centimeters).
	 *
	 * @param maxDistance Maximum distance in centimeters.
	 */
	void SetMaxAllowedDistance(uint32_t maxDistance);

	/**
	 * @brief Get the maximum allowed distance for UWB ranging.
	 *
	 * @return Maximum distance in centimeters.
	 */
	uint32_t GetMaxAllowedDistance();

	/**
	 * @brief Handles the ranging session data.
	 *
	 * @param sessionContext The session context.
	 * @param uwbData The ranging session data.
	 */
	void HandleRangingSessionData(SessionContext sessionContext, const UwbRangingData &uwbData);

	/**
	 * @brief Handles the ranging session state change.
	 *
	 * @param sessionContext The session context.
	 * @param state The ranging session state.
	 */
	void HandleRangingSessionStateChanged(SessionContext sessionContext, RangingSessionState state);

	/**
	 * @brief Handles the session termination.
	 *
	 * @param sessionContext The session context.
	 * @param isNfcSession Indicates if the session is a NFC session.
	 */
	void HandleSessionTermination(SessionContext sessionContext, bool isNfcSession);

private:
	AccessManagerImpl *Impl();
	const AccessManagerImpl *Impl() const;
};

/**
 * @brief Get the singleton instance of the implementation of the AccessManager.
 *
 * @return Reference to the singleton instance.
 */
extern AccessManager &AccessManagerInstance();

} // namespace Aliro

#include "access_manager_impl.h"
