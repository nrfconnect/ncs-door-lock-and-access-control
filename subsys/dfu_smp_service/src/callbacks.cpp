/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/mgmt/mcumgr/grp/img_mgmt/img_mgmt.h>
#include <zephyr/mgmt/mcumgr/mgmt/callbacks.h>
#include <zephyr/mgmt/mcumgr/mgmt/mgmt.h>

#include <zephyr/logging/log.h>

LOG_MODULE_DECLARE(DfuSmpService, CONFIG_DOOR_LOCK_DFU_SMP_SERVICE_LOG_LEVEL);

namespace DoorLock::DfuSmpService::Callbacks {

namespace {

mgmt_cb_return UploadConfirmHandler(uint32_t, mgmt_cb_return, int32_t *, uint16_t *, bool *, void *data, size_t)
{
	const img_mgmt_upload_check &imgData = *static_cast<img_mgmt_upload_check *>(data);

	LOG_DBG("DFU over SMP progress: %u/%u B of image: %u", static_cast<unsigned>(imgData.req->off),
		static_cast<unsigned>(imgData.action->size), static_cast<unsigned>(imgData.req->image));
	return MGMT_CB_OK;
}

mgmt_callback sUploadCallback{
	.callback = UploadConfirmHandler,
	.event_id = MGMT_EVT_OP_IMG_MGMT_DFU_CHUNK,
};

} // namespace

void Init()
{
	mgmt_callback_register(&sUploadCallback);
}

} // namespace DoorLock::DfuSmpService::Callbacks
