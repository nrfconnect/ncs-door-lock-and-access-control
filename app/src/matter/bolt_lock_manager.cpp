/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "bolt_lock_manager.h"
#include "access_manager/access_manager.h"
#include "app/task_executor.h"

#include "aliro/aliro.h"

#ifdef CONFIG_DOOR_LOCK_STEP_UP_PHASE
#include "validity_iterations.h"
#endif // CONFIG_DOOR_LOCK_STEP_UP_PHASE

#include "app_task.h"

using namespace chip;

BoltLockManager BoltLockManager::sLock;

namespace {

[[maybe_unused]] Aliro::OperationSource ToAliroOperationSource(BoltLockManager::OperationSource operationSource)
{
	switch (operationSource) {
	case BoltLockManager::OperationSource::kManual:
	case BoltLockManager::OperationSource::kKeypad:
	case BoltLockManager::OperationSource::kButton:
	case BoltLockManager::OperationSource::kRfid:
	case BoltLockManager::OperationSource::kBiometric:
		return Aliro::OperationSource::Manual;
	case BoltLockManager::OperationSource::kRemote:
		return Aliro::OperationSource::Matter;
	case BoltLockManager::OperationSource::kAuto:
		return Aliro::OperationSource::Auto;
	case BoltLockManager::OperationSource::kSchedule:
		return Aliro::OperationSource::Schedule;
	default:
		return Aliro::OperationSource::Unspecified;
	}
}

} // namespace

void BoltLockManager::Init(StateChangeCallback callback)
{
	mStateChangeCallback = callback;

	mLockSim.Init([](Aliro::OperationSource, Aliro::ReaderStateByte state) {
		Nrf::PostTask([state] { BoltLockMgr().UpdateState(state); });
	});

	// Set Aliro AccessManager application callbacks
	Aliro::AccessManagerInstance().SetApplicationCallbacks({
		.mUnlockIndicatorClb =
			[](Aliro::OperationSource source) {
				Nrf::PostTask([source] {
					if (!BoltLockMgr().Unlock(source)) {
#ifdef CONFIG_DOOR_LOCK_BLE_UWB
						// The lock is already unlocked, so we can send the Unsecured state
						Aliro::AliroStack::Instance().SendReaderStatusChangedMessage(
							source, Aliro::ReaderStateByte::Unsecured);
#endif // CONFIG_DOOR_LOCK_BLE_UWB
					}
				});
			},
		.mLockIndicatorClb =
			[](Aliro::OperationSource source) { Nrf::PostTask([source] { BoltLockMgr().Lock(source); }); },
	});

	auto addPublicKey = [](uint16_t credentialIndex, CredentialTypeEnum credentialType,
			       chip::ByteSpan credentialData) {
		Aliro::CryptoTypes::PublicKey publicKey{};
		std::copy_n(credentialData.data(), publicKey.size(), publicKey.data());

		if (credentialType == CredentialTypeEnum::kAliroEvictableEndpointKey ||
		    credentialType == CredentialTypeEnum::kAliroNonEvictableEndpointKey) {
			Aliro::AccessManagerInstance().AddPublicKey(
				publicKey, Aliro::AccessManager::PublicKeyType::AccessCredential, credentialIndex);
		} else if (credentialType == CredentialTypeEnum::kAliroCredentialIssuerKey) {
			Aliro::AccessManagerInstance().AddPublicKey(
				publicKey, Aliro::AccessManager::PublicKeyType::CredentialIssuer, credentialIndex);
		}
	};

	auto removePublicKey = []([[maybe_unused]] uint16_t credentialIndex, CredentialTypeEnum credentialType,
				  chip::ByteSpan credentialData) {
		Aliro::CryptoTypes::PublicKey publicKey{};
		std::copy_n(credentialData.data(), publicKey.size(), publicKey.data());

		if (credentialType == CredentialTypeEnum::kAliroEvictableEndpointKey ||
		    credentialType == CredentialTypeEnum::kAliroNonEvictableEndpointKey) {
			Aliro::AccessManagerInstance().RemovePublicKey(
				Aliro::AccessManager::PublicKeyType::AccessCredential, credentialIndex);
		} else if (credentialType == CredentialTypeEnum::kAliroCredentialIssuerKey) {
			Aliro::AccessManagerInstance().RemovePublicKey(
				Aliro::AccessManager::PublicKeyType::CredentialIssuer, credentialIndex);

#ifdef CONFIG_DOOR_LOCK_STEP_UP_PHASE
			Aliro::ClearValidityIterations(credentialIndex);
#endif // CONFIG_DOOR_LOCK_STEP_UP_PHASE
		}
	};

	AccessMgr::Instance().Init(addPublicKey, removePublicKey, nullptr, addPublicKey);

	/* Set the default state */
	Nrf::GetBoard().GetLED(Nrf::DeviceLeds::LED2).Set(IsLocked());
}

bool BoltLockManager::GetUser(uint16_t userIndex, EmberAfPluginDoorLockUserInfo &user)
{
	return AccessMgr::Instance().GetUserInfo(userIndex, user);
}

bool BoltLockManager::SetUser(uint16_t userIndex, FabricIndex creator, FabricIndex modifier, const CharSpan &userName,
			      uint32_t uniqueId, UserStatusEnum userStatus, UserTypeEnum userType,
			      CredentialRuleEnum credentialRule, const CredentialStruct *credentials,
			      size_t totalCredentials)
{
	return AccessMgr::Instance().SetUser(userIndex, creator, modifier, userName, uniqueId, userStatus, userType,
					     credentialRule, credentials, totalCredentials);
}

bool BoltLockManager::GetCredential(uint16_t credentialIndex, CredentialTypeEnum credentialType,
				    EmberAfPluginDoorLockCredentialInfo &credential)
{
	return AccessMgr::Instance().GetCredential(credentialIndex, credentialType, credential);
}

bool BoltLockManager::SetCredential(uint16_t credentialIndex, FabricIndex creator, FabricIndex modifier,
				    DlCredentialStatus credentialStatus, CredentialTypeEnum credentialType,
				    const ByteSpan &secret)
{
	return AccessMgr::Instance().SetCredential(credentialIndex, creator, modifier, credentialStatus, credentialType,
						   secret);
}

#ifdef CONFIG_LOCK_SCHEDULES

DlStatus BoltLockManager::GetWeekDaySchedule(uint8_t weekdayIndex, uint16_t userIndex,
					     EmberAfPluginDoorLockWeekDaySchedule &schedule)
{
	return AccessMgr::Instance().GetWeekDaySchedule(weekdayIndex, userIndex, schedule);
}

DlStatus BoltLockManager::SetWeekDaySchedule(uint8_t weekdayIndex, uint16_t userIndex, DlScheduleStatus status,
					     DaysMaskMap daysMask, uint8_t startHour, uint8_t startMinute,
					     uint8_t endHour, uint8_t endMinute)
{
	return AccessMgr::Instance().SetWeekDaySchedule(weekdayIndex, userIndex, status, daysMask, startHour,
							startMinute, endHour, endMinute);
}

DlStatus BoltLockManager::GetYearDaySchedule(uint8_t yearDayIndex, uint16_t userIndex,
					     EmberAfPluginDoorLockYearDaySchedule &schedule)
{
	return AccessMgr::Instance().GetYearDaySchedule(yearDayIndex, userIndex, schedule);
}

DlStatus BoltLockManager::SetYearDaySchedule(uint8_t yeardayIndex, uint16_t userIndex, DlScheduleStatus status,
					     uint32_t localStartTime, uint32_t localEndTime)
{
	return AccessMgr::Instance().SetYearDaySchedule(yeardayIndex, userIndex, status, localStartTime, localEndTime);
}

DlStatus BoltLockManager::GetHolidaySchedule(uint8_t holidayIndex, EmberAfPluginDoorLockHolidaySchedule &schedule)
{
	return AccessMgr::Instance().GetHolidaySchedule(holidayIndex, schedule);
}

DlStatus BoltLockManager::SetHolidaySchedule(uint8_t holidayIndex, DlScheduleStatus status, uint32_t localStartTime,
					     uint32_t localEndTime, OperatingModeEnum operatingMode)
{
	return AccessMgr::Instance().SetHolidaySchedule(holidayIndex, status, localStartTime, localEndTime,
							operatingMode);
}

#endif /* CONFIG_LOCK_SCHEDULES */

bool BoltLockManager::ValidatePIN(const Optional<chip::ByteSpan> &pinCode, OperationErrorEnum &err,
				  Nullable<ValidatePINResult> &result)
{
	return AccessMgr::Instance().ValidatePIN(pinCode, err, result);
}

void BoltLockManager::SetRequirePIN(bool require)
{
	return AccessMgr::Instance().SetRequirePIN(require);
}
bool BoltLockManager::GetRequirePIN()
{
	return AccessMgr::Instance().GetRequirePIN();
}

void BoltLockManager::Lock(const OperationSource source, const Nullable<chip::FabricIndex> &fabricIdx,
			   const Nullable<chip::NodeId> &nodeId, const Nullable<ValidatePINResult> &validatePINResult)
{
	VerifyOrReturn(mStateData.mState != Aliro::ReaderStateByte::Secured);
	mStateData = { Aliro::ReaderStateByte::EnteringSecured,
		       source,
		       ToAliroOperationSource(source),
		       fabricIdx,
		       nodeId,
		       validatePINResult };

	mLockSim.Lock(ToAliroOperationSource(source));
}

void BoltLockManager::Unlock(const OperationSource source, const Nullable<chip::FabricIndex> &fabricIdx,
			     const Nullable<chip::NodeId> &nodeId, const Nullable<ValidatePINResult> &validatePINResult)
{
	VerifyOrReturn(mStateData.mState != Aliro::ReaderStateByte::Unsecured);
	mStateData = { Aliro::ReaderStateByte::EnteringUnsecured,
		       source,
		       ToAliroOperationSource(source),
		       fabricIdx,
		       nodeId,
		       validatePINResult };

	mLockSim.Unlock(ToAliroOperationSource(source));
}

bool BoltLockManager::Lock(Aliro::OperationSource source)
{
	VerifyOrReturnValue(mStateData.mState != Aliro::ReaderStateByte::Secured, false);
	mStateData = { Aliro::ReaderStateByte::EnteringSecured,
		       OperationSource::kAliro,
		       source,
		       NullNullable,
		       NullNullable,
		       NullNullable };

	mLockSim.Lock(source);
	return true;
}

bool BoltLockManager::Unlock(Aliro::OperationSource source)
{
	VerifyOrReturnValue(mStateData.mState != Aliro::ReaderStateByte::Unsecured, false);
	mStateData = { Aliro::ReaderStateByte::EnteringUnsecured,
		       OperationSource::kAliro,
		       source,
		       NullNullable,
		       NullNullable,
		       NullNullable };

	mLockSim.Unlock(source);
	return true;
}

void BoltLockManager::UpdateState(Aliro::ReaderStateByte state)
{
	mStateData.mState = state;

	if (mStateChangeCallback != nullptr) {
		mStateChangeCallback(mStateData);
	}
}

void BoltLockManager::FactoryReset()
{
	AccessMgr::Instance().FactoryReset();
}
