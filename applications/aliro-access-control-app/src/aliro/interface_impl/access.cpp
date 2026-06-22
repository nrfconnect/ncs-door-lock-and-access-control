/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "aliro/interface.h"

#include "aliro/access_manager/access_manager.h"

#ifdef CONFIG_DOOR_LOCK_EXPEDITED_FAST_PHASE
#include "aliro/kpersistent_manager/kpersistent_manager_impl.h"
#endif // CONFIG_DOOR_LOCK_EXPEDITED_FAST_PHASE

#ifdef CONFIG_DOOR_LOCK_EXPEDITED_FAST_PHASE
extern Aliro::KpersistentManagerImpl sKpersistentManagerImpl;
#endif // CONFIG_DOOR_LOCK_EXPEDITED_FAST_PHASE

namespace Aliro::Interface::Access {

std::optional<AccessDocumentRequestParams>
GetAccessDocumentRequestParameters(const CryptoTypes::PublicKey &publicKey,
				   const std::optional<Timestamp> &credentialSignedTimestamp)
{
	return AccessManagerInstance().ShouldRequestAccessDocument(publicKey, credentialSignedTimestamp);
}

AliroError ProcessAccessRequest(ConnectionHandle handle, const CryptoTypes::PublicKey &userPublicKey,
				CryptoTypes::KeyId kpersistentKeyId,
				const std::optional<AccessDocumentTypes::AccessDocument> &accessDocument)
{
	return AccessManagerInstance().VerifyAccessCredential(userPublicKey, handle, kpersistentKeyId, accessDocument);
}

AliroError ProcessAccessRequest(ConnectionHandle handle, CryptoTypes::KeyId kpersistentKeyId)
{
	return AccessManagerInstance().VerifyKPersistentKey(kpersistentKeyId, handle);
}

AliroError GetCredentialIssuerPublicKey(const CryptoTypes::KeyIdentifier &keyIdentifier,
					CryptoTypes::PublicKey &publicKey)
{
	return AccessManagerInstance().GetCredentialIssuerPublicKey(keyIdentifier, publicKey);
}

AliroError GetKpersistentCount(size_t &count)
{
#ifdef CONFIG_DOOR_LOCK_EXPEDITED_FAST_PHASE
	return sKpersistentManagerImpl.GetKpersistentCount(count);
#else
	ARG_UNUSED(count);
	return ALIRO_ERROR_NOT_IMPLEMENTED;
#endif // CONFIG_DOOR_LOCK_EXPEDITED_FAST_PHASE
}

AliroError GetKpersistentKeyIds(CryptoTypes::KeyId *keyIds, size_t &count)
{
#ifdef CONFIG_DOOR_LOCK_EXPEDITED_FAST_PHASE
	return sKpersistentManagerImpl.GetKpersistentKeyIds(keyIds, count);
#else
	ARG_UNUSED(keyIds);
	ARG_UNUSED(count);
	return ALIRO_ERROR_NOT_IMPLEMENTED;
#endif // CONFIG_DOOR_LOCK_EXPEDITED_FAST_PHASE
}

AliroError GetAccessCredentialPublicKey(CryptoTypes::KeyId kpersistentKeyId, CryptoTypes::PublicKey &publicKey)
{
#ifdef CONFIG_DOOR_LOCK_EXPEDITED_FAST_PHASE
	return sKpersistentManagerImpl.GetAccessCredentialPublicKey(kpersistentKeyId, publicKey);
#else
	ARG_UNUSED(kpersistentKeyId);
	ARG_UNUSED(publicKey);
	return ALIRO_ERROR_NOT_IMPLEMENTED;
#endif // CONFIG_DOOR_LOCK_EXPEDITED_FAST_PHASE
}

} // namespace Aliro::Interface::Access
