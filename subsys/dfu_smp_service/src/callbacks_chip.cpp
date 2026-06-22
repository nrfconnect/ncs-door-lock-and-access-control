/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "callbacks.h"

#include <lib/support/CodeUtils.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/nrfconnect/DFUSync.h>
#include <platform/nrfconnect/ExternalFlashManager.h>

#include <zephyr/bluetooth/conn.h>

#include <zephyr/mgmt/mcumgr/grp/img_mgmt/img_mgmt.h>
#include <zephyr/mgmt/mcumgr/mgmt/callbacks.h>
#include <zephyr/mgmt/mcumgr/mgmt/mgmt.h>

#include <zephyr/logging/log.h>

LOG_MODULE_DECLARE(DfuSmpService, CONFIG_DOOR_LOCK_DFU_SMP_SERVICE_LOG_LEVEL);

namespace DoorLock::DfuSmpService::Callbacks {

namespace {

bool sDfuInProgress;
uint32_t sDfuSyncMutexId;

CHIP_ERROR TakeDfuSyncMutex()
{
	chip::DeviceLayer::PlatformMgr().LockChipStack();
	const auto err = DFUSync::GetInstance().Take(sDfuSyncMutexId);
	chip::DeviceLayer::PlatformMgr().UnlockChipStack();
	return err;
}

CHIP_ERROR FreeDfuSyncMutex()
{
	chip::DeviceLayer::PlatformMgr().LockChipStack();
	const auto err = DFUSync::GetInstance().Free(sDfuSyncMutexId);
	chip::DeviceLayer::PlatformMgr().UnlockChipStack();
	return err;
}

mgmt_cb_return UploadConfirmHandler(uint32_t, mgmt_cb_return, int32_t *rc, uint16_t *, bool *, void *data, size_t)
{
	const img_mgmt_upload_check &imgData = *static_cast<img_mgmt_upload_check *>(data);

	if (!sDfuInProgress) {
		if (TakeDfuSyncMutex() == CHIP_NO_ERROR) {
			sDfuInProgress = true;
		} else {
			LOG_ERR("Cannot start DFU over SMP, another DFU in progress.");
			*rc = MGMT_ERR_EBUSY;
			return MGMT_CB_ERROR_RC;
		}
	}

	LOG_DBG("DFU over SMP progress: %u/%u B of image: %u", static_cast<unsigned>(imgData.req->off),
		static_cast<unsigned>(imgData.action->size), static_cast<unsigned>(imgData.req->image));
	return MGMT_CB_OK;
}

mgmt_callback sUploadCallback{
	.callback = UploadConfirmHandler,
	.event_id = MGMT_EVT_OP_IMG_MGMT_DFU_CHUNK,
};

enum mgmt_cb_return DfuStoppedHandler(uint32_t, enum mgmt_cb_return, int32_t *, uint16_t *, bool *, void *, size_t)
{
	FreeDfuSyncMutex();

	sDfuInProgress = false;

	return MGMT_CB_OK;
}

mgmt_callback sDfuStopped = {
	.callback = DfuStoppedHandler,
	.event_id = (MGMT_EVT_OP_IMG_MGMT_DFU_STOPPED | MGMT_EVT_OP_IMG_MGMT_DFU_PENDING),
};

void Disconnected(bt_conn *conn, [[maybe_unused]] uint8_t reason)
{
	VerifyOrReturn(sDfuInProgress);

	bt_conn_info btInfo;
	VerifyOrReturn(bt_conn_get_info(conn, &btInfo) == 0);

	// Ignore other identities
	VerifyOrReturn(btInfo.id == BT_ID_DEFAULT);

	// Drop all callbacks incoming for the role other than peripheral, required by the Matter accessory
	VerifyOrReturn(btInfo.role == BT_CONN_ROLE_PERIPHERAL);

	sDfuInProgress = false;
	FreeDfuSyncMutex();
}

BT_CONN_CB_DEFINE(sConnCallbacks) = {
	.disconnected = Disconnected,
};

#ifdef CONFIG_CHIP_QSPI_NOR_POWER_MANAGEMENT_SUPPORT

enum mgmt_cb_return CommandHandler(uint32_t event, enum mgmt_cb_return, int32_t *, uint16_t *, bool *, void *, size_t)
{
	if (event == MGMT_EVT_OP_CMD_RECV) {
		chip::DeviceLayer::ExternalFlashManager::GetInstance().DoAction(
			chip::DeviceLayer::ExternalFlashManager::Action::WAKE_UP);
	} else if (event == MGMT_EVT_OP_CMD_DONE) {
		chip::DeviceLayer::ExternalFlashManager::GetInstance().DoAction(
			chip::DeviceLayer::ExternalFlashManager::Action::SLEEP);
	}

	return MGMT_CB_OK;
}

mgmt_callback sCommandCallback = {
	.callback = CommandHandler,
	.event_id = (MGMT_EVT_OP_CMD_RECV | MGMT_EVT_OP_CMD_DONE),
};

#endif // CONFIG_CHIP_QSPI_NOR_POWER_MANAGEMENT_SUPPORT

} // namespace

void Init()
{
	mgmt_callback_register(&sUploadCallback);
	mgmt_callback_register(&sDfuStopped);

#ifdef CONFIG_CHIP_QSPI_NOR_POWER_MANAGEMENT_SUPPORT
	mgmt_callback_register(&sCommandCallback);
#endif // CONFIG_CHIP_QSPI_NOR_POWER_MANAGEMENT_SUPPORT
}

} // namespace DoorLock::DfuSmpService::Callbacks
