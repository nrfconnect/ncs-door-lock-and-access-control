/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

#include "access_manager.h"

namespace Aliro {

AccessManagerImpl *AccessManager::Impl()
{
	return static_cast<AccessManagerImpl *>(this);
}

const AccessManagerImpl *AccessManager::Impl() const
{
	return static_cast<const AccessManagerImpl *>(this);
}

void AccessManager::SetApplicationCallbacks(const ApplicationCallbacks &callbacks)
{
	Impl()->_SetApplicationCallbacks(callbacks);
}

std::optional<Interface::Access::AccessDocumentRequestParams>
AccessManager::ShouldRequestAccessDocument(const CryptoTypes::PublicKey &publicKey,
					   const std::optional<Timestamp> &credentialSignedTimestamp)
{
	return Impl()->_ShouldRequestAccessDocument(publicKey, credentialSignedTimestamp);
}

AliroError
AccessManager::VerifyAccessCredential(const CryptoTypes::PublicKey &userPublicKey, SessionContext sessionContext,
				      CryptoTypes::KeyId kpersistentKeyId,
				      const std::optional<AccessDocumentTypes::AccessDocument> &accessDocument)
{
	return Impl()->_VerifyAccessCredential(userPublicKey, sessionContext, kpersistentKeyId, accessDocument);
}

AliroError AccessManager::VerifyKPersistentKey(CryptoTypes::KeyId kpersistentKeyId, SessionContext sessionContext)
{
	return Impl()->_VerifyKPersistentKey(kpersistentKeyId, sessionContext);
}

#ifdef CONFIG_NCS_ALIRO_BLE_UWB
AliroError AccessManager::StartRangingSession(uint32_t rangingSessionId, const CryptoTypes::Ursk &ursk,
					      ProtocolVersion protocolVersion, SessionContext sessionContext)
{
	return Impl()->_StartRangingSession(rangingSessionId, ursk, protocolVersion, sessionContext);
}
#endif // CONFIG_NCS_ALIRO_BLE_UWB

AliroError AccessManager::AddPublicKey(const CryptoTypes::PublicKey &publicKey, PublicKeyType publicKeyType,
				       size_t keyIndex)
{
	return Impl()->_AddPublicKey(publicKey, publicKeyType, keyIndex);
}

bool AccessManager::IsPublicKeyStored(const CryptoTypes::PublicKey &publicKey, size_t *keyIndex)
{
	return Impl()->_IsPublicKeyStored(publicKey, keyIndex);
}

AliroError AccessManager::GetPublicKey(size_t keyIndex, CryptoTypes::PublicKey &publicKey)
{
	return Impl()->_GetPublicKey(keyIndex, publicKey);
}

AliroError AccessManager::RemovePublicKey(PublicKeyType publicKeyType, size_t keyIndex)
{
	return Impl()->_RemovePublicKey(publicKeyType, keyIndex, false);
}

AliroError AccessManager::GetCredentialIssuerPublicKey(const CryptoTypes::KeyIdentifier &keyIdentifier,
						       CryptoTypes::PublicKey &publicKey) const
{
	return Impl()->_GetCredentialIssuerPublicKey(keyIdentifier, publicKey);
}

void AccessManager::ClearStoredKeys()
{
	return Impl()->_ClearStoredKeys();
}

void AccessManager::SetMaxAllowedDistance(uint32_t maxDistance)
{
	return Impl()->_SetMaxAllowedDistance(maxDistance);
}

uint32_t AccessManager::GetMaxAllowedDistance()
{
	return Impl()->_GetMaxAllowedDistance();
}

void AccessManager::HandleRangingSessionData(SessionContext sessionContext, const UwbRangingData &uwbData)
{
	return Impl()->_HandleRangingSessionData(sessionContext, uwbData);
}

void AccessManager::HandleRangingSessionStateChanged(SessionContext sessionContext, RangingSessionState state)
{
	return Impl()->_HandleRangingSessionStateChanged(sessionContext, state);
}

void AccessManager::HandleSessionTermination(SessionContext sessionContext)
{
	return Impl()->_HandleSessionTermination(sessionContext);
}

} // namespace Aliro
