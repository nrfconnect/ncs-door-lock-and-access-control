
/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include "access_manager/access_manager.h"

namespace Aliro {

/**
 * @brief A template for custom access manager implementation.
 *
 * This class is a template for custom Access Manager implementation.
 * It can be used as a base to implement customized Access Manager.
 */
class AccessManagerImpl : public AccessManager {
private:
	friend class AccessManager;

	void _SetApplicationCallbacks(const ApplicationCallbacks &callbacks);
	void _SetStackCallbacks(const StackCallbacks &callbacks);
	bool _ShouldRequestAccessDocument(const CryptoTypes::PublicKey &userPublicKey);
	AliroError _VerifyAccessCredential(
		const CryptoTypes::PublicKey &userPublicKey, bool isNfcSession, SessionContext sessionContext,
		const std::optional<AccessDocumentTypes::AccessDocument> &accessDocument = std::nullopt);
	AliroError _VerifyKPersistentKey(CryptoTypes::KeyId kpersistentKeyId, bool isNfcSession,
					 SessionContext sessionContext);
#ifdef CONFIG_ALIRO_BLE_UWB
	AliroError _StartRangingSession(uint32_t rangingSessionId, const CryptoTypes::Ursk &ursk,
					SessionContext sessionContext);
#endif // CONFIG_ALIRO_BLE_UWB
	AliroError _AddPublicKey(const CryptoTypes::PublicKey &publicKey, PublicKeyType publicKeyType,
				 std::optional<size_t> keyIndex = std::nullopt);
	bool _IsPublicKeyStored(const CryptoTypes::PublicKey &publicKey, size_t *keyIndex);
	AliroError _GetPublicKey(size_t keyIndex, CryptoTypes::PublicKey &publicKey);
	AliroError _RemovePublicKey(const CryptoTypes::PublicKey &publicKey, PublicKeyType publicKeyType);
	AliroError _GetCredentialIssuerPublicKey(const CryptoTypes::KeyIdentifier &keyIdentifier,
						 CryptoTypes::PublicKey &publicKey) const;
	void _ClearStoredKeys();
	void _SetMaxAllowedDistance(uint32_t maxDistance);
	uint32_t _GetMaxAllowedDistance();
	void _HandleRangingSessionData(SessionContext sessionContext, const UwbRangingData &uwbData);
	void _HandleRangingSessionStateChanged(SessionContext sessionContext, RangingSessionState state);
	void _HandleSessionTermination(SessionContext sessionContext);
};

inline AccessManager &AccessManagerInstance()
{
	static AccessManagerImpl sInstance{};
	return sInstance;
}

} // namespace Aliro
