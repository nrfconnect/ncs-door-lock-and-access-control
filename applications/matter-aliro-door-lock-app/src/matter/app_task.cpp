/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "app_task.h"
#include "bolt_lock_manager.h"
#include "clusters/identify.h"

#include "app/matter_init.h"
#include "app/task_executor.h"

#ifdef CONFIG_DOOR_LOCK_DFU_SMP_SERVICE
#include <dfu_smp_service/dfu_smp_service.h>
#endif // CONFIG_DOOR_LOCK_DFU_SMP_SERVICE

#ifdef CONFIG_DOOR_LOCK_NUS_SERVICE
#include <nus_service/nus_service.h>
#endif // CONFIG_DOOR_LOCK_NUS_SERVICE

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/clusters/door-lock-server/door-lock-server.h>
#include <platform/CHIPDeviceLayer.h>
#include <setup_payload/OnboardingCodesUtil.h>

#include <aliro/aliro.h>
#include <aliro/aliro_state_control.h>
#include <aliro/init.h>

#include <zephyr/logging/log.h>

LOG_MODULE_DECLARE(app, CONFIG_CHIP_APP_LOG_LEVEL);

using namespace ::chip;
using namespace ::chip::app;
using namespace ::chip::DeviceLayer;

namespace {
constexpr EndpointId kLockEndpointId{ 1 };

#if defined(CONFIG_DOOR_LOCK_NUS_SERVICE) || defined(CONFIG_DOOR_LOCK_DFU_SMP_SERVICE)

enum class AdvertisingPriority : uint8_t {
	DFU_SMP = CHIP_DEVICE_BLE_ADVERTISING_PRIORITY + 1,
	NUS,
};

constexpr uint16_t kAdvertisingIntervalMin{ 400 };
constexpr uint16_t kAdvertisingIntervalMax{ 500 };

#endif // CONFIG_DOOR_LOCK_NUS_SERVICE || CONFIG_DOOR_LOCK_DFU_SMP_SERVICE

#define APPLICATION_BUTTON_MASK DK_BTN2_MSK
#define DFU_SMP_BUTTON_MASK DK_BTN3_MSK

#ifndef CONFIG_CHIP_FACTORY_RESET_ERASE_SETTINGS
void AppEventHandler(const ChipDeviceEvent *event, [[maybe_unused]] intptr_t)
{
	constexpr bool reinitializeStorage{ !IS_ENABLED(CONFIG_CHIP_LAST_FABRIC_REMOVED_ERASE_AND_REBOOT) };

	switch (event->Type) {
	case DeviceEventType::kFactoryReset:
		// With this configuration we have to manually clean up the storage,
		// as whole settings partition won't be erased.
		BoltLockMgr().FactoryReset();
		ClearStorageAliro(reinitializeStorage);
		break;
	default:
		break;
	}
}
#endif /* CONFIG_CHIP_FACTORY_RESET_ERASE_SETTINGS */

Nrf::Matter::IdentifyCluster sIdentifyCluster(kLockEndpointId, false, []() {
	Nrf::PostTask([] { Nrf::GetBoard().GetLED(Nrf::DeviceLeds::LED2).Set(BoltLockMgr().IsLocked()); });
});

} /* namespace */

void AppTask::ButtonEventHandler(Nrf::ButtonState state, Nrf::ButtonMask hasChanged)
{
	if ((APPLICATION_BUTTON_MASK & hasChanged) & state) {
		Nrf::PostTask([] { LockActionEventHandler(); });
	}

#ifdef CONFIG_DOOR_LOCK_DFU_SMP_SERVICE
	if ((DFU_SMP_BUTTON_MASK & hasChanged) & state) {
		Nrf::PostTask([] { DfuSmpActionEventHandler(); });
	}
#endif // CONFIG_DOOR_LOCK_DFU_SMP_SERVICE
}

void AppTask::LockActionEventHandler()
{
	if (BoltLockMgr().IsLocked()) {
		BoltLockMgr().Unlock(BoltLockManager::OperationSource::kButton);
	} else {
		BoltLockMgr().Lock(BoltLockManager::OperationSource::kButton);
	}
}

#ifdef CONFIG_DOOR_LOCK_DFU_SMP_SERVICE
void AppTask::DfuSmpActionEventHandler()
{
	DoorLock::DfuSmpService::Toggle();
}
#endif // CONFIG_DOOR_LOCK_DFU_SMP_SERVICE

void AppTask::LockStateChanged(const BoltLockManager::StateData &stateData)
{
	switch (stateData.mState) {
	case Aliro::ReaderStateByte::EnteringSecured:
		LOG_INF("Lock action initiated");
		Nrf::GetBoard().GetLED(Nrf::DeviceLeds::LED2).Blink(50, 50);
#ifdef CONFIG_DOOR_LOCK_NUS_SERVICE
		DoorLock::NUSService::Send("locking");
#endif // CONFIG_DOOR_LOCK_NUS_SERVICE
		break;
	case Aliro::ReaderStateByte::Secured:
		LOG_INF("Lock action completed");
		Nrf::GetBoard().GetLED(Nrf::DeviceLeds::LED2).Set(true);
#ifdef CONFIG_DOOR_LOCK_NUS_SERVICE
		DoorLock::NUSService::Send("locked");
#endif // CONFIG_DOOR_LOCK_NUS_SERVICE
		break;
	case Aliro::ReaderStateByte::EnteringUnsecured:
		LOG_INF("Unlock action initiated");
		Nrf::GetBoard().GetLED(Nrf::DeviceLeds::LED2).Blink(50, 50);
#ifdef CONFIG_DOOR_LOCK_NUS_SERVICE
		DoorLock::NUSService::Send("unlocking");
#endif // CONFIG_DOOR_LOCK_NUS_SERVICE
		break;
	case Aliro::ReaderStateByte::Unsecured:
		LOG_INF("Unlock action completed");
		Nrf::GetBoard().GetLED(Nrf::DeviceLeds::LED2).Set(false);
#ifdef CONFIG_DOOR_LOCK_NUS_SERVICE
		DoorLock::NUSService::Send("unlocked");
#endif // CONFIG_DOOR_LOCK_NUS_SERVICE
		break;
	default:
		break;
	}

#ifdef CONFIG_DOOR_LOCK_BLE_UWB

	Aliro::AliroStack::Instance().SendReaderStatusChangedMessage(stateData.mAliroSource, stateData.mState);

#endif

	/* Handle changing attribute state in the application */
	Instance().UpdateClusterState(stateData);
}

void AppTask::UpdateClusterState(const BoltLockManager::StateData &stateData)
{
	BoltLockManager::StateData *stateDataCopy = Platform::New<BoltLockManager::StateData>(stateData);

	if (stateDataCopy == nullptr) {
		LOG_ERR("Failed to allocate memory for BoltLockManager::StateData");
		return;
	}

	CHIP_ERROR err = SystemLayer().ScheduleLambda([stateDataCopy]() {
		UpdateClusterStateHandler(*stateDataCopy);
		Platform::Delete(stateDataCopy);
	});

	if (err != CHIP_NO_ERROR) {
		LOG_ERR("Failed to schedule lambda: %" CHIP_ERROR_FORMAT, err.Format());
		Platform::Delete(stateDataCopy);
	}
}

void AppTask::UpdateClusterStateHandler(const BoltLockManager::StateData &stateData)
{
	using namespace chip::app::Clusters::DoorLock::Attributes;

	DlLockState newLockState;

	switch (stateData.mState) {
	case Aliro::ReaderStateByte::Secured:
		newLockState = DlLockState::kLocked;
		break;
	case Aliro::ReaderStateByte::Unsecured:
		newLockState = DlLockState::kUnlocked;
		break;
	default:
		newLockState = DlLockState::kNotFullyLocked;
		break;
	}

	Nullable<DlLockState> currentLockState;
	LockState::Get(kLockEndpointId, currentLockState);

	if (currentLockState.IsNull()) {
		/* Initialize lock state with start value, but not invoke lock/unlock. */
		LockState::Set(kLockEndpointId, newLockState);
	} else {
		LOG_INF("Updating LockState attribute");

		Nullable<uint16_t> userId;
		Nullable<List<const LockOpCredentials>> credentials;
#ifdef CONFIG_LOCK_PASS_CREDENTIALS_TO_SET_LOCK_STATE
		List<const LockOpCredentials> credentialList;
#endif

		if (!stateData.mValidatePINResult.IsNull()) {
			userId = { stateData.mValidatePINResult.Value().mUserId };

#ifdef CONFIG_LOCK_PASS_CREDENTIALS_TO_SET_LOCK_STATE
			/* `DoorLockServer::SetLockState` exptects list of `LockOpCredentials`,
			   however in case of PIN validation it makes no sense to have more than one
			   credential corresponding to validation result. For simplicity we wrap single
			   credential in list here. */
			credentialList = { &stateData.mValidatePINResult.Value().mCredential, 1 };
			credentials = { credentialList };
#endif
		}

		if (!DoorLockServer::Instance().SetLockState(kLockEndpointId, newLockState, stateData.mSource, userId,
							     credentials, stateData.mFabricIdx, stateData.mNodeId)) {
			LOG_ERR("Failed to update LockState attribute");
		}
	}
}

#ifdef CONFIG_DOOR_LOCK_NUS_SERVICE
void AppTask::NUSLockCallback(void *context)
{
	LOG_DBG("Received LOCK command from NUS");
	if (BoltLockMgr().GetState().mState == Aliro::ReaderStateByte::Secured ||
	    BoltLockMgr().GetState().mState == Aliro::ReaderStateByte::EnteringSecured) {
		LOG_INF("Device is already locked");
	} else {
		Nrf::PostTask([] { LockActionEventHandler(); });
	}
}

void AppTask::NUSUnlockCallback(void *context)
{
	LOG_DBG("Received UNLOCK command from NUS");
	if (BoltLockMgr().GetState().mState == Aliro::ReaderStateByte::Unsecured ||
	    BoltLockMgr().GetState().mState == Aliro::ReaderStateByte::EnteringUnsecured) {
		LOG_INF("Device is already unlocked");
	} else {
		Nrf::PostTask([] { LockActionEventHandler(); });
	}
}
#endif // CONFIG_DOOR_LOCK_NUS_SERVICE

#ifdef CONFIG_NCS_SAMPLE_MATTER_TEST_EVENT_TRIGGERS
CHIP_ERROR AppTask::DoorLockJammedEventCallback(Nrf::Matter::TestEventTrigger::TriggerValue)
{
	VerifyOrReturnError(DoorLockServer::Instance().SendLockAlarmEvent(kLockEndpointId, AlarmCodeEnum::kLockJammed),
			    CHIP_ERROR_INTERNAL);
	LOG_ERR("Event Trigger: Doorlock jammed.");
	return CHIP_NO_ERROR;
}
#endif

CHIP_ERROR AppTask::Init()
{
	Nrf::Matter::InitData initData{};

#ifdef CONFIG_DOOR_LOCK_DFU_SMP_SERVICE
	initData.mPostServerInitClbk = []() {
		DoorLock::DfuSmpService::Init(to_underlying(AdvertisingPriority::DFU_SMP), kAdvertisingIntervalMin,
					      kAdvertisingIntervalMax);
		DoorLock::DfuSmpService::ConfirmNewImage();
		return CHIP_NO_ERROR;
	};
#endif // CONFIG_DOOR_LOCK_DFU_SMP_SERVICE

	/* Initialize Matter stack */
	ReturnErrorOnFailure(Nrf::Matter::PrepareServer(initData));

	if (!Nrf::GetBoard().Init(ButtonEventHandler)) {
		LOG_ERR("User interface initialization failed.");
		return CHIP_ERROR_INCORRECT_STATE;
	}

	/* Register Matter event handler that controls the connectivity status LED based on the captured Matter network
	 * state. */
	ReturnErrorOnFailure(Nrf::Matter::RegisterEventHandler(Nrf::Board::DefaultMatterEventHandler, 0));

#ifndef CONFIG_CHIP_FACTORY_RESET_ERASE_SETTINGS
	/* Register application event handler. */
	ReturnErrorOnFailure(Nrf::Matter::RegisterEventHandler(AppEventHandler, 0));
#endif /* CONFIG_CHIP_FACTORY_RESET_ERASE_SETTINGS */

	/* Initialize lock manager */
	BoltLockMgr().Init(LockStateChanged);

#ifdef CONFIG_DOOR_LOCK_NUS_SERVICE
	int err = DoorLock::NUSService::Init();
	VerifyOrReturnError(err == 0, CHIP_ERROR_INTERNAL, LOG_ERR("Failed to initialize NUS service"));
	DoorLock::NUSService::RegisterCommand("Lock", NUSLockCallback, nullptr);
	DoorLock::NUSService::RegisterCommand("Unlock", NUSUnlockCallback, nullptr);
	err = DoorLock::NUSService::Start(to_underlying(AdvertisingPriority::NUS), kAdvertisingIntervalMin,
					  kAdvertisingIntervalMax);
	VerifyOrReturnError(err == 0, CHIP_ERROR_INTERNAL, LOG_ERR("Failed to start NUS service"));
#endif // CONFIG_DOOR_LOCK_NUS_SERVICE

	/* Register Door Lock test event trigger */
#ifdef CONFIG_NCS_SAMPLE_MATTER_TEST_EVENT_TRIGGERS
	ReturnErrorOnFailure(Nrf::Matter::TestEventTrigger::Instance().RegisterTestEventTrigger(
		kDoorLockJammedEventTriggerId,
		Nrf::Matter::TestEventTrigger::EventTrigger{ 0, DoorLockJammedEventCallback }));
#endif

	ReturnErrorOnFailure(sIdentifyCluster.Init());

	return Nrf::Matter::StartServer();
}

CHIP_ERROR AppTask::StartApp()
{
	ReturnErrorOnFailure(Init());
	VerifyOrReturnError(AliroInit() == EXIT_SUCCESS, CHIP_ERROR_INTERNAL, LOG_ERR("Failed to initialize Aliro"));
	VerifyOrReturnError(DoorLock::AliroStateControl::UpdateAliroState() == ALIRO_NO_ERROR, CHIP_ERROR_INTERNAL,
			    LOG_ERR("Failed to update Aliro state"));

	while (true) {
		Nrf::DispatchNextTask();
	}

	return CHIP_NO_ERROR;
}
