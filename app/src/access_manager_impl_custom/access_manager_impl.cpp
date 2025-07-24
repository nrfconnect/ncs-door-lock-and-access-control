/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "access_manager_impl.h"

#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(access_manager_impl_custom, CONFIG_NCS_DOOR_LOCK_APP_LOG_LEVEL);

namespace Aliro {

AliroError AccessManagerImpl::_Init(const Callbacks &callbacks)
{
	LOG_INF("AccessManagerImpl custom init");
	return ALIRO_ERROR_NOT_IMPLEMENTED;
}

AliroError AccessManagerImpl::_StartAccessDecision(const CryptoTypes::PublicKey &userPublicKey,
						   SessionContext sessionContext)
{
	LOG_INF("AccessManagerImpl custom start access decision for NFC session");
	return ALIRO_ERROR_NOT_IMPLEMENTED;
}

#ifdef CONFIG_ALIRO_BLE_TP
AliroError AccessManagerImpl::_StartAccessDecision(const CryptoTypes::PublicKey &userPublicKey,
						   uint32_t rangingSessionId, const CryptoTypes::Ursk &ursk,
						   SessionContext sessionContext)
{
	LOG_INF("AccessManagerImpl custom start access decision for BLE session");
	return ALIRO_ERROR_NOT_IMPLEMENTED;
}
#endif // CONFIG_ALIRO_BLE_TP

AliroError AccessManagerImpl::_AddPublicKey(const CryptoTypes::PublicKey &publicKey)
{
	LOG_INF("AccessManagerImpl custom add public key");
	return ALIRO_ERROR_NOT_IMPLEMENTED;
}

AliroError AccessManagerImpl::_RemovePublicKey(const CryptoTypes::PublicKey &publicKey)
{
	LOG_INF("AccessManagerImpl custom remove public key");
	return ALIRO_ERROR_NOT_IMPLEMENTED;
}

void AccessManagerImpl::_ClearStoredKeys()
{
	LOG_INF("AccessManagerImpl custom clear stored keys");
}

void AccessManagerImpl::_SetMaxAllowedDistance(uint32_t maxDistance)
{
	LOG_INF("AccessManagerImpl custom set max allowed distance");
}

uint32_t AccessManagerImpl::_GetMaxAllowedDistance()
{
	LOG_INF("AccessManagerImpl custom get max allowed distance");
	return 0;
}

void AccessManagerImpl::_HandleRangingSessionData(const UwbRangingData &uwbData)
{
	LOG_INF("AccessManagerImpl custom handle ranging session data");
}

void AccessManagerImpl::_HandleSessionTermination(SessionContext sessionContext)
{
	LOG_INF("AccessManagerImpl custom handle session termination");
}

} // namespace Aliro
