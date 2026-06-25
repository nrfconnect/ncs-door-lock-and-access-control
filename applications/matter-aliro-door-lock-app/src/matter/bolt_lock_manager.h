/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include "aliro/types.h"

#include <app/clusters/door-lock-server/door-lock-server.h>
#include <lib/core/ClusterEnums.h>
#include <matter_access/access_manager.h>

#include <zephyr/kernel.h>

#include <cstdint>

#include <lock_sim.h>

class BoltLockManager {
	using AccessMgr = DoorLock::MatterAccess::AccessManagerType;

public:
	using OperationSource = chip::app::Clusters::DoorLock::OperationSourceEnum;
	using ValidateCredentialResult = AccessMgr::ValidateCredentialResult;

	struct StateData {
		Aliro::ReaderStateByte mState;
		OperationSource mSource;
		Aliro::OperationSource mAliroSource;
		Nullable<chip::FabricIndex> mFabricIdx;
		Nullable<chip::NodeId> mNodeId;
		Nullable<ValidateCredentialResult> mValidateCredentialResult;
	};

	using StateChangeCallback = void (*)(const StateData &);

	void Init(StateChangeCallback callback);

	const StateData &GetState() const { return mStateData; }
	bool IsLocked() const { return mStateData.mState == Aliro::ReaderStateByte::Secured; }

	bool GetUser(uint16_t userIndex, EmberAfPluginDoorLockUserInfo &user);
	bool SetUser(uint16_t userIndex, chip::FabricIndex creator, chip::FabricIndex modifier,
		     const chip::CharSpan &userName, uint32_t uniqueId, UserStatusEnum userStatus,
		     UserTypeEnum userType, CredentialRuleEnum credentialRule, const CredentialStruct *credentials,
		     size_t totalCredentials);

	bool GetCredential(uint16_t credentialIndex, CredentialTypeEnum credentialType,
			   EmberAfPluginDoorLockCredentialInfo &credential);
	bool SetCredential(uint16_t credentialIndex, chip::FabricIndex creator, chip::FabricIndex modifier,
			   DlCredentialStatus credentialStatus, CredentialTypeEnum credentialType,
			   const chip::ByteSpan &secret);

#ifdef CONFIG_DOOR_LOCK_MATTER_ACCESS_SCHEDULES
	DlStatus GetWeekDaySchedule(uint8_t weekdayIndex, uint16_t userIndex,
				    EmberAfPluginDoorLockWeekDaySchedule &schedule);
	DlStatus SetWeekDaySchedule(uint8_t weekdayIndex, uint16_t userIndex, DlScheduleStatus status,
				    DaysMaskMap daysMask, uint8_t startHour, uint8_t startMinute, uint8_t endHour,
				    uint8_t endMinute);
	DlStatus GetYearDaySchedule(uint8_t yearDayIndex, uint16_t userIndex,
				    EmberAfPluginDoorLockYearDaySchedule &schedule);
	DlStatus SetYearDaySchedule(uint8_t yearDayIndex, uint16_t userIndex, DlScheduleStatus status,
				    uint32_t localStartTime, uint32_t localEndTime);
	DlStatus GetHolidaySchedule(uint8_t holidayIndex, EmberAfPluginDoorLockHolidaySchedule &schedule);
	DlStatus SetHolidaySchedule(uint8_t holidayIndex, DlScheduleStatus status, uint32_t localStartTime,
				    uint32_t localEndTime, OperatingModeEnum operatingMode);
#endif /* CONFIG_DOOR_LOCK_MATTER_ACCESS_SCHEDULES */

	bool ValidateCredential(CredentialTypeEnum credentialType, const chip::ByteSpan &secret,
				OperationErrorEnum &error, Nullable<ValidateCredentialResult> &result);

	void Lock(const OperationSource source, const Nullable<chip::FabricIndex> &fabricIdx = NullNullable,
		  const Nullable<chip::NodeId> &nodeId = NullNullable,
		  const Nullable<ValidateCredentialResult> &validateCredentialResult = NullNullable);
	void Unlock(const OperationSource source, const Nullable<chip::FabricIndex> &fabricIdx = NullNullable,
		    const Nullable<chip::NodeId> &nodeId = NullNullable,
		    const Nullable<ValidateCredentialResult> &validateCredentialResult = NullNullable);

	bool Lock(Aliro::OperationSource source, const Nullable<ValidateCredentialResult> &validateCredentialResult);
	bool Unlock(Aliro::OperationSource source, const Nullable<ValidateCredentialResult> &validateCredentialResult);

#ifdef CONFIG_DOOR_LOCK_MATTER_ACCESS_CREDENTIAL_TYPES_PIN
	void SetRequirePIN(bool require);
	bool GetRequirePIN();
#endif // CONFIG_DOOR_LOCK_MATTER_ACCESS_CREDENTIAL_TYPES_PIN

	void FactoryReset();

private:
	friend class AppTask;

	void UpdateState(Aliro::ReaderStateByte state);

	friend BoltLockManager &BoltLockMgr();

	StateData mStateData = {
		Aliro::ReaderStateByte::Secured, OperationSource::kButton, Aliro::OperationSource::Manual, {}, {}, {}
	};
	StateChangeCallback mStateChangeCallback = nullptr;

	Aliro::LockSim mLockSim;

	static BoltLockManager sLock;
};

inline BoltLockManager &BoltLockMgr()
{
	return BoltLockManager::sLock;
}
