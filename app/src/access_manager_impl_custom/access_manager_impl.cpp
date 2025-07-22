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

AliroError AccessManagerImpl::_StartAccessDecision(const CryptoTypes::PublicKey &userPublicKey, bool isBleSession)
{
	LOG_INF("AccessManagerImpl custom start access decision");
	return ALIRO_ERROR_NOT_IMPLEMENTED;
}

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

uint32_t AccessManagerImpl::_GetMaxAllowedDistance() const
{
	LOG_INF("AccessManagerImpl custom get max allowed distance");
	return 0;
}

void AccessManagerImpl::_HandleRangingSessionData(const UwbRangingData &uwbData)
{
	LOG_INF("AccessManagerImpl custom handle ranging session data");
}

} // namespace Aliro
