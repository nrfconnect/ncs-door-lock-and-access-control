/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "access_manager_impl.h"

#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(access_manager_impl_custom, CONFIG_NCS_DOOR_LOCK_APP_LOG_LEVEL);

namespace Aliro {

AliroError AccessManagerImpl::_Init(const ApplicationCallbacks &)
{
	LOG_INF("AccessManagerImpl custom init");
	return ALIRO_ERROR_NOT_IMPLEMENTED;
}

void AccessManagerImpl::_SetStackCallbacks(const StackCallbacks &)
{
	LOG_INF("AccessManagerImpl custom set stack callbacks");
}

AliroError AccessManagerImpl::_VerifyAccessCredential(const CryptoTypes::PublicKey &, bool, SessionContext)
{
	LOG_INF("AccessManagerImpl custom verify access credential");
	return ALIRO_ERROR_NOT_IMPLEMENTED;
}

#ifdef CONFIG_ALIRO_BLE_UWB
AliroError AccessManagerImpl::_StartRangingSession(uint32_t, const CryptoTypes::Ursk &, SessionContext)
{
	LOG_INF("AccessManagerImpl custom start ranging session");
	return ALIRO_ERROR_NOT_IMPLEMENTED;
}
#endif // CONFIG_ALIRO_BLE_UWB

AliroError AccessManagerImpl::_AddPublicKey(const CryptoTypes::PublicKey &)
{
	LOG_INF("AccessManagerImpl custom add public key");
	return ALIRO_ERROR_NOT_IMPLEMENTED;
}

AliroError AccessManagerImpl::_RemovePublicKey(const CryptoTypes::PublicKey &)
{
	LOG_INF("AccessManagerImpl custom remove public key");
	return ALIRO_ERROR_NOT_IMPLEMENTED;
}

void AccessManagerImpl::_ClearStoredKeys()
{
	LOG_INF("AccessManagerImpl custom clear stored keys");
}

void AccessManagerImpl::_SetMaxAllowedDistance(uint32_t)
{
	LOG_INF("AccessManagerImpl custom set max allowed distance");
}

uint32_t AccessManagerImpl::_GetMaxAllowedDistance()
{
	LOG_INF("AccessManagerImpl custom get max allowed distance");
	return 0;
}

void AccessManagerImpl::_HandleRangingSessionData(SessionContext, const UwbRangingData &)
{
	LOG_INF("AccessManagerImpl custom handle ranging session data");
}

void AccessManagerImpl::_HandleSessionTermination(SessionContext)
{
	LOG_INF("AccessManagerImpl custom handle session termination");
}

} // namespace Aliro
