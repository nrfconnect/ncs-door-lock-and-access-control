/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "access_manager_impl.h"

#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(access_manager_impl_custom, CONFIG_NCS_DOOR_LOCK_APP_LOG_LEVEL);

namespace Aliro {

void AccessManagerImpl::_SetApplicationCallbacks(const ApplicationCallbacks &)
{
	LOG_INF("Custom %s function", __FUNCTION__);
}

void AccessManagerImpl::_SetStackCallbacks(const StackCallbacks &)
{
	LOG_INF("Custom %s function", __FUNCTION__);
}

bool AccessManagerImpl::_ShouldRequestAccessDocument(const CryptoTypes::PublicKey &)
{
	LOG_INF("Custom %s function", __FUNCTION__);
	return false;
}

AliroError AccessManagerImpl::_VerifyAccessCredential(const CryptoTypes::PublicKey &, bool, SessionContext,
						      const std::optional<AccessDocumentTypes::AccessDocument> &)
{
	LOG_INF("Custom %s function", __FUNCTION__);
	return ALIRO_ERROR_NOT_IMPLEMENTED;
}

AliroError AccessManagerImpl::_VerifyKPersistentKey(CryptoTypes::KeyId, bool, SessionContext)
{
	LOG_INF("Custom %s function", __FUNCTION__);
	return ALIRO_ERROR_NOT_IMPLEMENTED;
}

#ifdef CONFIG_ALIRO_BLE_UWB
AliroError AccessManagerImpl::_StartRangingSession(uint32_t, const CryptoTypes::Ursk &, SessionContext)
{
	LOG_INF("Custom %s function", __FUNCTION__);
	return ALIRO_ERROR_NOT_IMPLEMENTED;
}
#endif // CONFIG_ALIRO_BLE_UWB

AliroError AccessManagerImpl::_AddPublicKey(const CryptoTypes::PublicKey &, PublicKeyType, std::optional<size_t>)
{
	LOG_INF("Custom %s function", __FUNCTION__);
	return ALIRO_ERROR_NOT_IMPLEMENTED;
}

AliroError AccessManagerImpl::_GetCredentialIssuerPublicKey(const CryptoTypes::KeyIdentifier &,
							    CryptoTypes::PublicKey &) const
{
	LOG_INF("Custom %s function", __FUNCTION__);
	return ALIRO_ERROR_NOT_IMPLEMENTED;
}

bool AccessManagerImpl::_IsPublicKeyStored(const CryptoTypes::PublicKey &, size_t *)
{
	LOG_INF("Custom %s function", __FUNCTION__);
	return false;
}

AliroError AccessManagerImpl::_GetPublicKey(size_t, CryptoTypes::PublicKey &)
{
	LOG_INF("Custom %s function", __FUNCTION__);
	return ALIRO_ERROR_NOT_IMPLEMENTED;
}

AliroError AccessManagerImpl::_RemovePublicKey(const CryptoTypes::PublicKey &, PublicKeyType)
{
	LOG_INF("Custom %s function", __FUNCTION__);
	return ALIRO_ERROR_NOT_IMPLEMENTED;
}

void AccessManagerImpl::_ClearStoredKeys()
{
	LOG_INF("Custom %s function", __FUNCTION__);
}

void AccessManagerImpl::_SetMaxAllowedDistance(uint32_t)
{
	LOG_INF("Custom %s function", __FUNCTION__);
}

uint32_t AccessManagerImpl::_GetMaxAllowedDistance()
{
	LOG_INF("Custom %s function", __FUNCTION__);
	return 0;
}

void AccessManagerImpl::_HandleRangingSessionData(SessionContext, const UwbRangingData &)
{
	LOG_INF("Custom %s function", __FUNCTION__);
}

void AccessManagerImpl::_HandleSessionTermination(SessionContext)
{
	LOG_INF("Custom %s function", __FUNCTION__);
}

} // namespace Aliro
