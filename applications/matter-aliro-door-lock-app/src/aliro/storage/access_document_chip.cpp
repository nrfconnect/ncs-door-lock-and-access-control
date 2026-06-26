/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <aliro/errors.h>
#include <aliro/types.h>

#include <platform/CHIPDeviceLayer.h>

#include <matter_access/access_manager.h>

#include <zephyr/logging/log.h>

LOG_MODULE_DECLARE(access_document, CONFIG_DOOR_LOCK_APP_LOG_LEVEL);

namespace Aliro {

AliroError UpdateAliroEvictableCredential(size_t index, const CryptoTypes::PublicKey &publicKey,
					  size_t credentialIssuerKeyIndex)
{
	using AccessMgr = DoorLock::MatterAccess::AccessManagerType;

	const size_t credentialIndex = index + 1;
	const size_t credentialIssuerIndex = credentialIssuerKeyIndex + 1;

	chip::DeviceLayer::PlatformMgr().LockChipStack();
	const CHIP_ERROR error =
		AccessMgr::Instance().UpdateAliroEvictableCredential(credentialIndex, publicKey, credentialIssuerIndex);
	chip::DeviceLayer::PlatformMgr().UnlockChipStack();

	VerifyOrReturnValue(error == CHIP_NO_ERROR, ALIRO_ERROR_INTERNAL,
			    LOG_ERR("Failed to update Aliro evictable credential at index: %zu, error: %d", index,
				    error.AsInteger()));

	return ALIRO_NO_ERROR;
}

AliroError RemoveAliroEvictableCredential(size_t index, bool updateUser)
{
	using AccessMgr = DoorLock::MatterAccess::AccessManagerType;

	const size_t credentialIndex = index + 1;

	chip::DeviceLayer::PlatformMgr().LockChipStack();
	const CHIP_ERROR error = AccessMgr::Instance().RemoveAliroEvictableCredential(credentialIndex, updateUser);
	chip::DeviceLayer::PlatformMgr().UnlockChipStack();

	VerifyOrReturnValue(error == CHIP_NO_ERROR, ALIRO_ERROR_INTERNAL,
			    LOG_ERR("Failed to remove Aliro evictable credential at index: %zu, error: %d", index,
				    error.AsInteger()));

	return ALIRO_NO_ERROR;
}

} // namespace Aliro
