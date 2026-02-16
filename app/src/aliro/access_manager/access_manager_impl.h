
/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include "access_manager.h"
#include "aliro/kpersistent_manager/kpersistent_manager.h"
#include "aliro/types.h"

#ifdef CONFIG_DOOR_LOCK_ACCESS_MANAGER_TERMINATE_SESSION_ON_TIMEOUT
#include "aliro/timer.h"
#endif // CONFIG_DOOR_LOCK_ACCESS_MANAGER_TERMINATE_SESSION_ON_TIMEOUT

#ifdef CONFIG_DOOR_LOCK_STEP_UP_PHASE
#include "validity_iterations.h"
#endif // CONFIG_DOOR_LOCK_STEP_UP_PHASE

#include <zephyr/kernel.h>

#include <array>
#include <cstring>
#include <optional>

namespace Aliro {

class AccessManagerImpl final : public AccessManager {
public:
	void SetKpersistentManager(KpersistentManager *kpersistentManager) { mKpersistentManager = kpersistentManager; }

private:
	friend class AccessManager;
	friend AccessManager &AccessManagerInstance();
	friend AccessManagerImpl &AccessManagerInstanceImpl();

	/**
	 * @brief A template for storing keys.
	 *
	 * @tparam MaxKeys The maximum number of keys to store.
	 */
	template <size_t MaxKeys> struct StoredKeys {
		std::array<std::optional<CryptoTypes::PublicKey>, MaxKeys> mKeys{};
		size_t mCount{ 0 };
	};

	/**
	 * @brief Get an instance of the the AccessManagerImpl.
	 *
	 * @return Reference to the AccessManagerImpl instance.
	 */
	static AccessManagerImpl &Instance()
	{
		static AccessManagerImpl sInstance;
		return sInstance;
	}

	/**
	 * @brief Set the application callbacks for the AccessManager.
	 *
	 * @param callbacks Application callbacks.
	 */
	void _SetApplicationCallbacks(const ApplicationCallbacks &callbacks);

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
	_ShouldRequestAccessDocument(const CryptoTypes::PublicKey &publicKey,
				     const std::optional<Timestamp> &credentialSignedTimestamp);

	/**
	 * @brief Verifies the Access Credential based on provided inputs.
	 *
	 * @param userPublicKey The User Device public key to verify.
	 * @param sessionContext A pointer to the session context.
	 * @param kpersistentKeyId The volatile Kpersistent key generated during the Expedited-standard phase.
	 * @param accessDocument The access document provided by the User Device.
	 *
	 * @return ALIRO_NO_ERROR on success, error code otherwise.
	 */
	AliroError _VerifyAccessCredential(
		const CryptoTypes::PublicKey &userPublicKey, SessionContext sessionContext,
		CryptoTypes::KeyId kpersistentKeyId,
		const std::optional<AccessDocumentTypes::AccessDocument> &accessDocument = std::nullopt);

	/**
	 * @brief Verifies the Kpersistent key based on provided inputs.
	 *
	 * @param kpersistentKeyId The Kpersistent key ID to verify.
	 * @param sessionContext A pointer to the session context.
	 *
	 * @return ALIRO_NO_ERROR on success, error code otherwise.
	 */
	AliroError _VerifyKPersistentKey(CryptoTypes::KeyId kpersistentKeyId, SessionContext sessionContext);

#ifdef CONFIG_DOOR_LOCK_BLE_UWB
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
	AliroError _StartRangingSession(uint32_t rangingSessionId, const CryptoTypes::Ursk &ursk,
					ProtocolVersion protocolVersion, SessionContext sessionContext);
#endif // CONFIG_DOOR_LOCK_BLE_UWB

	/**
	 * @brief Add a new public key to the AccessManager.
	 *
	 * @param publicKey The public key to add.
	 * @param publicKeyType The type of the public key.
	 * @param keyIndex The index of the public key in the storage.
	 *
	 * @return ALIRO_NO_ERROR on success, error code on failure.
	 */
	AliroError _AddPublicKey(const CryptoTypes::PublicKey &publicKey, PublicKeyType publicKeyType, size_t keyIndex);

	/**
	 * @brief Check if a public key is stored in the AccessManager.
	 *
	 * @param publicKey The public key to check.
	 * @param keyIndex The index of the public key in the storage.
	 *
	 * @return True if the public key is stored, false otherwise.
	 */
	bool _IsPublicKeyStored(const CryptoTypes::PublicKey &publicKey, size_t *keyIndex = nullptr);

	/**
	 * @brief Get a public key from the AccessManager by its index.
	 *
	 * @param keyIndex The index of the public key in the storage.
	 * @param publicKey The public key to get.
	 *
	 * @return ALIRO_NO_ERROR on success, error code on failure.
	 */
	AliroError _GetPublicKey(size_t keyIndex, CryptoTypes::PublicKey &publicKey);

	/**
	 * @brief Remove a public key from the AccessManager.
	 *
	 * @param publicKeyType The type of the public key.
	 * @param keyIndex The index of the public key in the storage.
	 *
	 * @return ALIRO_NO_ERROR on success, error code on failure.
	 */
	AliroError _RemovePublicKey(PublicKeyType publicKeyType, size_t keyIndex);

	/**
	 * @brief Get a Credential Issuer public key by its identifier.
	 *
	 * @param keyIdentifier The key identifier of the Credential Issuer public key.
	 * @param publicKey The public key to get.
	 *
	 * @return ALIRO_NO_ERROR on success, error code on failure.
	 */
	AliroError _GetCredentialIssuerPublicKey(const CryptoTypes::KeyIdentifier &keyIdentifier,
						 CryptoTypes::PublicKey &publicKey) const;

	/**
	 * @brief Clear all stored public keys.
	 */
	void _ClearStoredKeys();

	/**
	 * @brief Set the maximum allowed distance for UWB ranging (in centimeters).
	 *
	 * @param maxDistance Maximum distance in centimeters.
	 */
	void _SetMaxAllowedDistance(uint32_t maxDistance);

	/**
	 * @brief Get the maximum allowed distance for UWB ranging.
	 *
	 * @return Maximum distance in centimeters.
	 */
	uint32_t _GetMaxAllowedDistance();

	/**
	 * @brief Handles the ranging session data.
	 *
	 * @param sessionContext The session context.
	 * @param uwbData The ranging session data.
	 */
	void _HandleRangingSessionData(SessionContext sessionContext, const UwbRangingData &uwbData);

	/**
	 * @brief Handles the UWB ranging session state change.
	 *
	 * @param sessionContext The session context.
	 * @param state The UWB ranging session state.
	 */
	void _HandleRangingSessionStateChanged(SessionContext sessionContext, RangingSessionState state);

	/**
	 * @brief Handles the session termination.
	 *
	 * @param sessionContext The session context.
	 */
	void _HandleSessionTermination(SessionContext sessionContext);

	AccessManagerImpl() = default;
	~AccessManagerImpl() = default;

	AccessManagerImpl(const AccessManagerImpl &) = delete;
	AccessManagerImpl &operator=(const AccessManagerImpl &) = delete;
	AccessManagerImpl(AccessManagerImpl &&) = delete;
	AccessManagerImpl &operator=(AccessManagerImpl &&) = delete;

	/**
	 * @brief Signals the access granted action.
	 *
	 * @param isNfcSession Indicates if the session is a NFC session.
	 *
	 * This callback is called when the User Device is authenticated and the access is granted.
	 */
	void AccessGrantedAction(bool isNfcSession) const;

	/**
	 * @brief Signals the access denied action.
	 *
	 * @param isNfcSession Indicates if the session is a NFC session.
	 *
	 * This callback is called when the User Device is not authenticated or the access is denied.
	 */
	void AccessDeniedAction(bool isNfcSession) const;

	/**
	 * @brief Signals the unlock action.
	 *
	 * This callback is called when access is granted and the door should be unlocked.
	 * In case of:
	 * - UWB ranging, this callback is called when the User Device is in range.
	 * - NFC, this callback is called just after the Access Credential is verified.
	 *
	 * @param isNfcSession Indicates if the session is a NFC session.
	 */
	void UnlockAction(bool isNfcSession) const;

	/**
	 * @brief Signals the lock action.
	 *
	 * This callback is called when the door should be locked.
	 * In case of:
	 * - UWB ranging, this callback is called when the User Device is out of range or BLE session is terminated.
	 *
	 * @param isNfcSession Indicates if the session is a NFC session.
	 */
	void LockAction(bool isNfcSession) const;

	/**
	 * @brief Template helper function to remove a key from a StoredKeys container.
	 *
	 * @param container The container to remove the key from.
	 * @param keyIndex The index of the public key in the storage.
	 *
	 * @return ALIRO_NO_ERROR on success, error code on failure.
	 */
	template <size_t T> AliroError RemoveKeyFromContainer(StoredKeys<T> &container, size_t keyIndex) const;

	/**
	 * @brief Add a new public key to a StoredKeys container.
	 *
	 * @param container The container to add the key to.
	 * @param publicKey The public key to add.
	 * @param keyIndex The index of the public key in the storage.
	 *
	 * @return ALIRO_NO_ERROR on success, error code on failure.
	 */
	template <size_t T>
	AliroError AddKeyToContainer(StoredKeys<T> &container, const CryptoTypes::PublicKey &publicKey,
				     size_t keyIndex) const;

	void HandleAccessGranted(bool isNfcSession, bool granted);
	bool VerifyPublicKey(const CryptoTypes::PublicKey &userPublicKey) const;
	bool ShouldUnlockImmediately(bool isNfcSession) const;

	template <size_t T>
	bool IsPublicKeyStored(const StoredKeys<T> &container, const CryptoTypes::PublicKey &userPublicKey,
			       size_t *keyIndex = nullptr) const;

	template <size_t T> AliroError GetFirstFreeIndex(StoredKeys<T> &container, size_t &keyIndex) const;

#ifdef CONFIG_DOOR_LOCK_STEP_UP_PHASE
	AliroError ProcessAccessDocument(const CryptoTypes::PublicKey &userPublicKey,
					 const AccessDocumentTypes::AccessDocument &accessDocument);
	AliroError RemoveOldestCredential(size_t &keyIndex);
	AliroError ProcessValidityIteration(const CryptoTypes::PublicKey &credentialIssuerPublicKey,
					    const std::optional<ValidityIteration> &validityIteration);
	AliroError UpdateValidityIteration(size_t credentialIssuerKeyIndex, const ValidityIterations &currentIterations,
					   ValidityIteration validityIteration);
	AliroError RemoveOldCredentials(size_t credentialIssuerKeyIndex, ValidityIteration validityIteration);
	AliroError RemoveAccessCredentials(size_t credentialIssuerKeyIndex);
#endif // CONFIG_DOOR_LOCK_STEP_UP_PHASE

#ifdef CONFIG_DOOR_LOCK_BLE_UWB
	struct RangingSessionContext {
#ifdef CONFIG_DOOR_LOCK_ACCESS_MANAGER_TERMINATE_SESSION_ON_TIMEOUT
		RangingSessionContext(uint32_t timeoutMs, SessionContext sessionContext)
			: mSessionContext(sessionContext),
			  mRangingSessionTimer(timeoutMs, RangingSessionTimerCallback, this)
		{
		}

		static void RangingSessionTimerCallback(Timer::Context ctx);
#else
		RangingSessionContext(SessionContext sessionContext) : mSessionContext(sessionContext) {}
#endif // CONFIG_DOOR_LOCK_ACCESS_MANAGER_TERMINATE_SESSION_ON_TIMEOUT

		sys_snode_t mNode{};
		SessionContext mSessionContext;
		bool mInRange{ false };
#ifdef CONFIG_DOOR_LOCK_ACCESS_MANAGER_TERMINATE_SESSION_ON_TIMEOUT
		Timer mRangingSessionTimer;
#endif // CONFIG_DOOR_LOCK_ACCESS_MANAGER_TERMINATE_SESSION_ON_TIMEOUT
	};

	bool AnalyzeUwbRangingData(const UwbRangingData &uwbData, SessionContext sessionContext);
	std::optional<uint16_t> ExtractDistanceFromUwbData(const UwbRangingData &uwbData) const;
	AliroError AddRangingSession(uint32_t rangingSessionId, const CryptoTypes::Ursk &ursk,
				     ProtocolVersion protocolVersion, const SessionContext sessionCtx);
	void RemoveRangingSession(SessionContext sessionCtx);
	RangingSessionContext *FindRangingSession(const SessionContext sessionCtx);
	std::optional<SessionContext> FindSessionContext(RangingSessionContext *rangingSessionCtx);
	bool IsUserDeviceInRange() const;
	void TerminateAliroSession(SessionContext sessionContext);
	void SetInRangeState(SessionContext sessionContext, bool sessionInRange, bool updateReaderState = true);

	static constexpr uint16_t kDefaultMaxAllowedDistance{ CONFIG_DOOR_LOCK_ACCESS_MANAGER_MAX_ALLOWED_DISTANCE_CM };
	static constexpr uint16_t kDefaultMaxAllowedDistanceExitMargin{
		CONFIG_DOOR_LOCK_ACCESS_MANAGER_MAX_ALLOWED_DISTANCE_EXIT_MARGIN_CM
	};

	// Maximum allowed distance for UWB ranging (in centimeters)
	uint32_t mMaxAllowedDistance{ kDefaultMaxAllowedDistance };
	// Exit margin above max allowed distance when door is unlocked (in centimeters)
	// Used only when door is unlocked to prevent rapid toggling at the threshold
	uint32_t mMaxAllowedDistanceExitMargin{ kDefaultMaxAllowedDistanceExitMargin };
	sys_slist_t mActiveSessions{};
#endif // CONFIG_DOOR_LOCK_BLE_UWB

	ApplicationCallbacks mCallbacks{};

	KpersistentManager *mKpersistentManager{ nullptr };

	StoredKeys<CONFIG_DOOR_LOCK_ACCESS_MANAGER_ACCESS_CREDENTIAL_MAX_STORED_KEYS> mAcKeys{};
	StoredKeys<CONFIG_DOOR_LOCK_ACCESS_MANAGER_CREDENTIAL_ISSUER_MAX_STORED_KEYS> mCiKeys{};
	StoredKeys<CONFIG_DOOR_LOCK_STORAGE_MAX_STORED_ACCESS_DOCUMENTS> mAdKeys{};
};

/**
 * @brief Get the singleton instance of the AccessManagerImpl.
 *
 * @return The instance of the AccessManagerImpl.
 */
inline AccessManager &AccessManagerInstance()
{
	return AccessManagerImpl::Instance();
}

/**
 * @brief Get the singleton instance of the AccessManagerImpl.
 *
 * @return The instance of the AccessManagerImpl.
 */
inline AccessManagerImpl &AccessManagerInstanceImpl()
{
	return AccessManagerImpl::Instance();
}

} // namespace Aliro
