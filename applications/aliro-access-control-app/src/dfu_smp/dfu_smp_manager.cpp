/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "dfu_smp_manager.h"

#include <advertising_arbiter/ble_advertising_arbiter.h>

#include <aliro/ble_types.h>
#include <aliro_workqueue/aliro_workqueue.h>
#include <doorlock/utils/utils.h>

#include <zephyr/dfu/mcuboot.h>
#include <zephyr/logging/log.h>
#include <zephyr/mgmt/mcumgr/grp/img_mgmt/img_mgmt.h>
#include <zephyr/mgmt/mcumgr/mgmt/callbacks.h>
#include <zephyr/mgmt/mcumgr/mgmt/mgmt.h>

#include <cstring>
#include <tuple>

LOG_MODULE_REGISTER(SmpManager, CONFIG_DFU_SMP_LOG_LEVEL);

namespace {

mgmt_cb_return UploadConfirmHandler(uint32_t, mgmt_cb_return, int32_t *, uint16_t *, bool *, void *data, size_t)
{
	const img_mgmt_upload_check &imgData = *static_cast<img_mgmt_upload_check *>(data);

	LOG_DBG("DFU over SMP progress: %u/%u B of image: %u", static_cast<unsigned>(imgData.req->off),
		static_cast<unsigned>(imgData.action->size), static_cast<unsigned>(imgData.req->image));
	return MGMT_CB_OK;
}

} // namespace

namespace Aliro::Dfu {

namespace BleArbiter = DoorLock::Interface::BleAdvertisingArbiter;

int SmpManager::InitButton()
{
	constexpr gpio_dt_spec button = GPIO_DT_SPEC_GET(DT_ALIAS(dfu_smp_button), gpios);

	VerifyOrReturnValue(device_is_ready(button.port), -ENODEV, LOG_ERR("SMP button GPIO device not ready"));

	int err = gpio_pin_configure_dt(&button, GPIO_INPUT);
	VerifyOrReturnValue(err == 0, err, LOG_ERR("Failed to configure SMP button GPIO"));

	err = gpio_pin_interrupt_configure_dt(&button, GPIO_INT_EDGE_TO_ACTIVE);
	VerifyOrReturnValue(err == 0, err, LOG_ERR("Failed to configure SMP button interrupt"));

	static gpio_callback buttonCb{};
	gpio_init_callback(
		&buttonCb, [](const device *, gpio_callback *, uint32_t) { Instance().Toggle(); }, BIT(button.pin));

	err = gpio_add_callback(button.port, &buttonCb);
	VerifyOrReturnValue(err == 0, err, LOG_ERR("Failed to add SMP button callback"));

	return 0;
}

void SmpManager::StartAdvertising()
{
	mRequest = BleArbiter::Request{ .mPriority = 0,
					.mOptions = BT_LE_ADV_OPT_CONN,
					.mMinInterval = BT_GAP_ADV_FAST_INT_MIN_2,
					.mMaxInterval = BT_GAP_ADV_FAST_INT_MAX_2 };

	// Set advertising data buffers
	mRequest.mAdvertisingData[0] = BT_DATA(BT_DATA_FLAGS, &kAdvertisingFlags, sizeof(kAdvertisingFlags));
	mRequest.mAdvertisingData[1] = BT_DATA(BT_DATA_UUID128_ALL, kSmpUuid.data(), kSmpUuid.size());

	const char *deviceName = bt_get_name();
	mRequest.mScanResponseData[0] =
		BT_DATA(BT_DATA_NAME_COMPLETE, deviceName, static_cast<uint8_t>(strlen(deviceName)));

	int err = BleArbiter::InsertRequest(mRequest);
	VerifyOrReturn(err == 0, LOG_ERR("DFU SMP advertising request failed (rc %d)", err));

	mIsAdvEnabled = true;
}

void SmpManager::StopAdvertising()
{
	BleArbiter::CancelRequest(mRequest);
	LOG_INF("DFU SMP advertising stopped");
	mIsAdvEnabled = false;
}

void SmpManager::Toggle()
{
	VerifyOrReturn(mIsInitialized, LOG_ERR("DFU SMP module not initialized"));
	std::ignore = AliroWorkqueueSubmit(&mWork);
}

int SmpManager::Init()
{
	static mgmt_callback sUploadCallback = {
		.callback = UploadConfirmHandler,
		.event_id = MGMT_EVT_OP_IMG_MGMT_DFU_CHUNK,
	};

	mgmt_callback_register(&sUploadCallback);

	k_work_init(&mWork, []([[maybe_unused]] k_work *) {
		if (Instance().IsSmpEnabled()) {
			Instance().StopAdvertising();
		} else {
			Instance().StartAdvertising();
		}
	});

	int err = InitButton();
	VerifyOrReturnValue(err == 0, err);

	mIsInitialized = true;

	LOG_DBG("DFU SMP module initialized");
	return 0;
}

void SmpManager::ConfirmNewImage()
{
	// Check if the image is run in the REVERT mode and eventually
	// confirm it to prevent reverting on the next boot.
	VerifyOrReturn(mcuboot_swap_type() == BOOT_SWAP_TYPE_REVERT);

	if (boot_write_img_confirmed()) {
		LOG_ERR("Failed to confirm firmware image, it will be reverted on the next boot");
	} else {
		LOG_DBG("New firmware image confirmed");
	}
}

} // namespace Aliro::Dfu
